#include "Panels/Panel.h"

#include <functional>
#include <limits>

#include <ion/math/transformutils.h>

#include "Agents/ActionAgent.h"
#include "Agents/BoardAgent.h"
#include "Agents/NameAgent.h"
#include "Agents/SelectionAgent.h"
#include "Agents/SessionAgent.h"
#include "Agents/SettingsAgent.h"
#include "Base/VirtualKeyboard.h"
#include "Items/Border.h"
#include "Math/Linear.h"
#include "Panels/DialogPanel.h"
#include "Panes/ButtonPane.h"
#include "Panes/TextPane.h"
#include "Place/ClickInfo.h"
#include "SG/ColorMap.h"
#include "SG/Search.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/Tuning.h"
#include "Widgets/ClickableWidget.h"
#include "Widgets/PushButtonWidget.h"

// ----------------------------------------------------------------------------
// Panel::Focuser_ class.
// ----------------------------------------------------------------------------

/// This class manages Pane focus for a Panel. It keeps track of all Panes that
/// can potentially receive focus and allows the focus to change in different
/// ways.
class Panel::Focuser_ {
  public:
    enum class Direction { kUp, kDown, kLeft, kRight };

    /// Function that returns true if a Pane can be focused.
    typedef std::function<bool(Pane &)> CanFocusFunc;

    /// Function to call when focus changes to a new Pane.
    typedef std::function<void(const PanePtr &)> ChangeFocusFunc;

    /// The constructor is passed a description string to use for log
    /// messages and a function to invoke to see if a sub-Pane can be focused.
    Focuser_(const Str &desc, const CanFocusFunc &can_focus_func,
             const ChangeFocusFunc &change_focus_func) :
        desc_(desc),
        can_focus_func_(can_focus_func),
        change_focus_func_(change_focus_func) {
        ASSERT(! desc.empty());
        ASSERT(can_focus_func);
        ASSERT(change_focus_func);
    }

    /// Sets the VirtualKeyboard instance to pass along to interactive Panes.
    void SetVirtualKeyboard(const VirtualKeyboardPtr &virtual_keyboard) {
        virtual_keyboard_ = virtual_keyboard;
    }

    /// Sets the interactive Pane instances that can potentially be focused.
    /// This also initializes the Panes for interaction.
    void SetPanes(const Pane::PaneVec &panes);

    /// Initializes focus if there is no Pane focused already.
    void InitFocus() {
        if (focused_index_ < 0)
            FocusFirstPane_();
    }

    /// Returns the focused Pane, which may be null.
    PanePtr GetFocusedPane() const {
        return focused_index_ >= 0 ? panes_[focused_index_] : PanePtr();
    }

    /// Sets focus on the given Pane, which must be one from SetPanes().
    void SetFocus(const PanePtr &pane);

    /// Changes focus in the given direction.
    void MoveFocus(Direction dir);

    /// Activates the given interactive Pane from a button click or key press.
    void ActivatePane(const PanePtr &pane, bool is_click);

  private:
    /// Border state saved for the focused Pane so it can be restored when
    /// losing focus.
    struct BorderState_ {
        bool  is_enabled;
        float width;
        Color color;
    };

    /// String used for log messages.
    const Str desc_;

    /// Function that returns true if a Pane can be focused.
    CanFocusFunc can_focus_func_;

    /// Function to invoke when a Pane is focused.
    ChangeFocusFunc change_focus_func_;

    /// VirtualKeyboard instance to pass to interactive Panes.
    VirtualKeyboardPtr virtual_keyboard_;

    /// All Panes in the Panel that are potentially able to receive focus.
    Pane::PaneVec panes_;

    /// Index into panes_ of the current Pane with focus. This is -1 if there
    /// is none.
    int focused_index_ = -1;

    /// Saves the previous state for the focused Pane.
    BorderState_ prev_state_;

    /// Initializes interaction for an interactive Pane.
    void InitPaneInteraction_(const PanePtr &pane);

    /// Finds the first Pane that can be focused, if there is one, and sets the
    /// focus on it.
    void FocusFirstPane_();

    /// Changes focus from \c old_pane to \c new_pane. Either may be null.
    void ChangeFocus_(const PanePtr &old_pane, const PanePtr &new_pane);
};

void Panel::Focuser_::SetPanes(const Pane::PaneVec &panes) {
    panes_ = panes;
    for (auto &pane: panes_)
        InitPaneInteraction_(pane);
}

void Panel::Focuser_::SetFocus(const PanePtr &pane) {
    ASSERT(pane);
    ASSERT(pane->GetInteractor());
    const PanePtr focused_pane = GetFocusedPane();

    // Do this even if the specified Pane is the same as focused_pane. This
    // allows some Panels (such as the FilePanel) to refocus on the same Pane
    // under certain circumstances.
    auto it = std::find(panes_.begin(), panes_.end(), pane);
    ASSERTM(it != panes_.end(), desc_ + " and " + pane->GetDesc());
    ChangeFocus_(focused_pane, pane);
    focused_index_ = it - panes_.begin();
}

void Panel::Focuser_::MoveFocus(Direction dir) {
    /// \todo Handle change in focus to left/right at some point.
    ASSERT(dir == Direction::kDown || dir == Direction::kUp);
    const int increment = dir == Direction::kDown ? 1 : -1;

    // Has to be a starting point.
    if (focused_index_ < 0)
        return;

    // Keep going in the specified direction until an enabled interactive pane
    // is found or the original focused pane is hit again.
    int new_index = focused_index_;
    while (true) {
        new_index = new_index + increment;
        if (new_index < 0)
            new_index = panes_.size() - 1;
        else if (static_cast<size_t>(new_index) >= panes_.size())
            new_index = 0;
        if (new_index == focused_index_)
            break;  // No other interactive pane.
        auto &pane = *panes_[new_index];
        if (can_focus_func_(pane))
            break;
    }

    if (new_index != focused_index_) {
        ChangeFocus_(panes_[focused_index_], panes_[new_index]);
        focused_index_ = new_index;
    }
}

void Panel::Focuser_::ActivatePane(const PanePtr &pane, bool is_click) {
    ASSERT(pane->GetInteractor());
    auto &interactor = *pane->GetInteractor();

    // Make sure the Pane is still able to interact. It may have been disabled
    // by another observer.
    if (! interactor.GetFocusBorder()) {
        MoveFocus(Focuser_::Direction::kDown);
        return;
    }

    // If the activation is not from a click, simulate a click on the
    // activation Widget, but do NOT invoke this again (infinite recursion kind
    // of thing).
    if (! is_click) {
        auto clickable = interactor.GetActivationWidget();
        ASSERT(clickable);
        clickable->GetActivation().EnableObserver(this, false);
        ClickInfo info;
        info.widget = clickable.get();
        clickable->Click(info);
        clickable->GetActivation().EnableObserver(this, true);
    }

    KLOG('F', desc_ << " activating " << pane->GetDesc());
    interactor.Activate();
}

void Panel::Focuser_::InitPaneInteraction_(const PanePtr &pane) {
    ASSERT(pane->GetInteractor());
    auto &interactor = *pane->GetInteractor();

    // Tell the IPaneInteractor about the VirtualKeyboard, if any.
    if (virtual_keyboard_)
        interactor.SetVirtualKeyboard(virtual_keyboard_);

    // If there is an activator Widget, observe its GetActivation() Notifier if
    // not already done. This is better than using the GetClicked() Notifier
    // because the observer should be notified at the start of a click or drag.
    if (auto clickable = interactor.GetActivationWidget()) {
        if (! clickable->GetActivation().HasObserver(this)) {
            auto func = [&, pane](Widget &, bool is_act){
                if (is_act) {
                    if (interactor.GetFocusBorder())
                        SetFocus(pane);
                    ActivatePane(pane, true);
                }
            };
            clickable->GetActivation().AddObserver(this, func);
        }
    }
}

void Panel::Focuser_::FocusFirstPane_() {
    // Use the first focusable interactive pane by default.
    ASSERT(focused_index_ < 0);
    for (size_t i = 0; i < panes_.size(); ++i) {
        if (panes_[i]->GetInteractor()->GetFocusBorder()) {
            focused_index_ = i;
            KLOG('F', desc_ << " focused on "
                 << panes_[0]->GetDesc() << " to start");
            ChangeFocus_(PanePtr(), panes_[i]);
            break;
        }
    }
}

void Panel::Focuser_::ChangeFocus_(const PanePtr &old_pane,
                                   const PanePtr &new_pane) {
    if (old_pane) {
        KLOG('F', desc_ << " removing focus from " << old_pane->GetDesc());
        if (auto border = old_pane->GetInteractor()->GetFocusBorder()) {
            border->SetEnabled(prev_state_.is_enabled);
            border->SetWidth(prev_state_.width);
            border->SetColor(prev_state_.color);
        }
    }
    if (new_pane) {
        KLOG('F', desc_ << " adding focus to " << new_pane->GetDesc());
        auto border = new_pane->GetInteractor()->GetFocusBorder();
        ASSERT(border);
        // Save state.
        prev_state_.is_enabled = border->IsEnabled();
        prev_state_.width      = border->GetWidth();
        prev_state_.color      = border->GetColor();
        // Activate.
        border->SetEnabled(true);
        border->SetWidth(TK::kFocusedPaneBorderWidth);
        border->SetColor(SG::ColorMap::SGetColor("PaneFocusColor"));

        change_focus_func_(new_pane);
    }
}

// ----------------------------------------------------------------------------
// Panel functions.
// ----------------------------------------------------------------------------

Panel::Panel() {
}

Panel::~Panel() {
}

void Panel::AddFields() {
    AddField(pane_.Init("pane"));
    AddField(is_movable_.Init("is_movable", true));
    AddField(is_resizable_.Init("is_resizable", false));

    SG::Node::AddFields();
}

bool Panel::IsValid(Str &details) {
    if (! SG::Node::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]

    if (! GetPane()) {
        details = "Missing Pane for " + GetDesc();
        return false;
    }

    return true;
}

void Panel::CreationDone() {
    SG::Node::CreationDone();

    if (! IsTemplate()) {
        auto    can_focus_func = [&](Pane &p){ return CanFocusPane_(p); };
        auto change_focus_func = [&](const PanePtr &p){ UpdateFocus(p); };
        focuser_.reset(
            new Focuser_(GetDesc(), can_focus_func, change_focus_func));

        // Add the root Pane as a child.
        ASSERTM(GetPane(), GetDesc());
        AddChild(GetPane());
    }
}

// LCOV_EXCL_START [only SetTestContext() is used in tests]
void Panel::SetContext(const ContextPtr &context) {
    // This should be called only once.
    ASSERT(! context_);

    // Verify that all needed instances exist.
    ASSERT(context);
    ASSERT(context->action_agent);
    ASSERT(context->board_agent);
    ASSERT(context->name_agent);
    ASSERT(context->selection_agent);
    ASSERT(context->session_agent);
    ASSERT(context->settings_agent);

    context_ = context;
    ProcessContext();
}
// LCOV_EXCL_STOP

void Panel::SetTestContext(const ContextPtr &context) {
    // Not all agents are necessarily needed for testing.
    ASSERT(context);
    context_ = context;
    ProcessContext();
}

void Panel::SetSize(const Vector2f &size) {
    // Can set the size only if it has not been set or the Panel is resizable.
    ASSERT(GetSize() == Vector2f::Zero() || IsResizable());
    if (auto pane = GetPane()) {
        pane->SetLayoutSize(size);
        size_may_have_changed_ = false;

        // Let the derived class know the Pane size may have changed.
        UpdateForPaneSizeChange();
    }
}

Vector2f Panel::GetSize() {
    auto pane = GetPane();
    if (size_may_have_changed_) {
        UpdateSize_();
        UpdateFocusablePanes();
        size_may_have_changed_ = false;
    }
    return pane->GetLayoutSize();
}

Vector2f Panel::GetMinSize() const {
    return GetPane()->GetBaseSize();
}

bool Panel::HandleEvent(const Event &event) {
    // If there is a VirtualKeyboard, track headset on/off events to set its
    // visibility.
    if (GetContext().virtual_keyboard &&
        (event.flags.Has(Event::Flag::kButtonPress) ||
         event.flags.Has(Event::Flag::kButtonRelease)) &&
        event.button == Event::Button::kHeadset) {
        GetContext().virtual_keyboard->SetIsVisible(
            event.flags.Has(Event::Flag::kButtonPress));
        return true;
    }

    bool handled = false;

    // Let the focused Pane (if any) handle the event.
    if (auto focused_pane = focuser_->GetFocusedPane()) {
        ASSERT(focused_pane->GetInteractor());
        handled = focused_pane->GetInteractor()->HandleEvent(event);
        if (handled) {
            KLOG('h', focused_pane->GetName() << " in " // LCOV_EXCL_LINE [bug]
                 << GetName() << " handled event");
        }
    }

    // Handle certain key presses.
    if (! handled && event.flags.Has(Event::Flag::kKeyPress))
        handled = ProcessKeyPress_(event);

    // If requested, handle all valuator events so the MainHandler does not get
    // them.
    if (! handled && event.flags.Has(Event::Flag::kPosition1D) &&
        ShouldTrapValuatorEvents())
        handled = true;

    return handled;
}

void Panel::SetStatus(Status status) {
    if (status_ != status) {
        KLOG('g', GetDesc() << " status now " << Util::EnumName(status));

        // If changing from unattached to visible, let the derived class update
        // any UI.
        if (status_ == Status::kUnattached && status == Status::kVisible) {
            UpdateInterface();
            focuser_->InitFocus();
        }
        status_ = status;
    }
}

void Panel::SetFocusedPane(const PanePtr &pane) {
    focuser_->SetFocus(pane);
}

PanePtr Panel::GetFocusedPane() const {
    return focuser_->GetFocusedPane();
}

WidgetPtr Panel::GetTouchedPaneWidget(const TouchInfo &info) {
    float closest_dist = std::numeric_limits<float>::max();
    return GetPane()->GetTouchedWidget(info, closest_dist);
}

void Panel::PostSetUpIon() {
    SG::Node::PostSetUpIon();

    // Let the derived class set up.
    InitInterface();

    // Detect root Pane base size changes.
    auto &root_pane = GetPane();
    root_pane->GetLayoutChanged().AddObserver(
        this, [&](){
            size_may_have_changed_ = true;
            size_changed_.Notify();
        });
}

void Panel::ProcessContext() {
    // The context is required for UpdateFocusablePanes() to work, so do it
    // now.
    UpdateFocusablePanes();
}

Panel::Context & Panel::GetContext() const {
    ASSERT(context_);
    return *context_;
}

const Settings & Panel::GetSettings() const {
    return GetContext().settings_agent->GetSettings();
}

void Panel::AddButtonFunc(const Str &name, const ButtonFunc &func) {
    auto but_pane = GetPane()->FindTypedSubPane<ButtonPane>(name);
    auto &clicked = but_pane->GetButton().GetClicked();
    clicked.AddObserver(this, [func](const ClickInfo &){ func(); });
}

void Panel::SetButtonText(const Str &name, const Str &text) {
    auto but_pane  = GetPane()->FindTypedSubPane<ButtonPane>(name);
    auto text_pane = but_pane->FindTypedSubPane<TextPane>("ButtonText");
    text_pane->SetText(text);
}

void Panel::EnableButton(const Str &name, bool enabled) {
    auto but_pane = GetPane()->FindTypedSubPane<ButtonPane>(name);
    but_pane->SetInteractionEnabled(enabled);
}

void Panel::UpdateFocusablePanes() {
    if (auto &vk = GetContext().virtual_keyboard)
        focuser_->SetVirtualKeyboard(vk);
    Pane::PaneVec panes;
    GetPane()->GetFocusableSubPanes(panes);
    focuser_->SetPanes(panes);
}

void Panel::SetFocus(const PanePtr &pane) {
    focuser_->SetFocus(pane);
}

void Panel::SetFocus(const Str &name) {
    SetFocus(GetPane()->FindSubPane(name));
}

void Panel::DisplayMessage(const Str &message) {
    auto dp = GetTypedPanel<DialogPanel>("DialogPanel");
    dp->SetMessage(message);
    dp->SetSingleResponse("OK");
    context_->board_agent->PushPanel(dp, nullptr);
}

void Panel::AskQuestion(const Str &question, const QuestionFunc &func,
                        bool is_no_default) {
    ASSERT(func);

    auto dp = GetTypedPanel<DialogPanel>("DialogPanel");
    dp->SetMessage(question);
    dp->SetChoiceResponse("No", "Yes", is_no_default);

    context_->board_agent->PushPanel(dp, func);
}

void Panel::Close(const Str &result) {
    context_->board_agent->ClosePanel(result);
}

PanelPtr Panel::GetPanel(const Str &name) const {
    return context_->board_agent->GetPanel(name);
}

void Panel::UpdateSize_() {
    auto pane = GetPane();

    // Do this until the base size is not changing any more.
    Vector2f size;
    do {
        // Make sure the layout size is at least as large as the base
        // size. This also makes sure the base sizes in all Panes are up to
        // date.
        size = MaxComponents(pane->GetBaseSize(), pane->GetLayoutSize());
        KLOG('p', GetDesc() << " updating size to " << size);

        // Lay out the Panes.
        pane->SetLayoutSize(size);
    }
    // Stop if the Pane has no more changes.
    while (pane->WasLayoutChanged());

    // Let the derived class know the Pane size may have changed.
    UpdateForPaneSizeChange();
}

bool Panel::ProcessKeyPress_(const Event &event) {
    const Str key_string = event.GetKeyString();
    bool handled = false;

    // Enter key activates the focused Pane (if any) if not already active.
    if (key_string == "Enter") {
        if (auto focused_pane = GetFocusedPane()) {
            ASSERT(focused_pane->GetInteractor());
            auto &interactor = *focused_pane->GetInteractor();
            if (! interactor.IsActive()) {
                focuser_->ActivatePane(focused_pane, false);
                KLOG('h', GetName() << " handled key press '" << key_string
                     << "' to activate " << focused_pane->GetName());
                handled = true;
            }
        }
    }

    // Canceling the Panel.
    else if (key_string == "Escape") {
        Close("Cancel");
        handled = true;
        KLOG('h', GetName() << " handled Escape key press");
    }

    // Navigation:
    else if (key_string == "Tab" || key_string == "Shift-Tab") {
        focuser_->MoveFocus(key_string == "Tab" ? Focuser_::Direction::kDown :
                            Focuser_::Direction::kUp);
        handled = true;
        KLOG('h', GetName() << " handled navigation key press");
    }

    return handled;
}

bool Panel::CanFocusPane_(Pane &pane) const {
    // The Pane has to have a focus border.
    if (! pane.GetInteractor()->GetFocusBorder())
        return false;

    // The Pane and all ancestors have to be enabled.
    for (const auto &p: SG::FindNodePathUnderNode(GetPane(), pane)) {
        if (! p->IsEnabled())
            return false;
    }

    return true;
}
