#include "Panels/Panel.h"

#include <functional>
#include <limits>

#include <ion/math/transformutils.h>

#include "Base/VirtualKeyboard.h"
#include "Items/Border.h"
#include "Locate/ClickInfo.h"
#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Managers/SessionManager.h"
#include "Managers/SettingsManager.h"
#include "Math/Intersection.h"
#include "Math/Linear.h"
#include "Panels/DialogPanel.h"
#include "Panes/ButtonPane.h"
#include "Panes/TextPane.h"
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

    /// The constructor is passed a description string to use for log
    /// messages.
    Focuser_(const std::string &desc) : desc_(desc) {}

    /// Sets the VirtualKeyboard instance to pass along to interactive Panes.
    void SetVirtualKeyboard(const VirtualKeyboardPtr &virtual_keyboard) {
        virtual_keyboard_ = virtual_keyboard;
    }

    /// Sets the interactive Pane instances that can potentially be focused.
    /// This also initializes the Panes for interaction.
    void SetPanes(const std::vector<PanePtr> &panes);

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
    const std::string desc_;

    /// VirtualKeyboard instance to pass to interactive Panes.
    VirtualKeyboardPtr virtual_keyboard_;

    /// All Panes in the Panel that are potentially able to receive focus.
    std::vector<PanePtr> panes_;

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

void Panel::Focuser_::SetPanes(const std::vector<PanePtr> &panes) {
    panes_ = panes;
    for (auto &pane: panes_)
        InitPaneInteraction_(pane);
}

void Panel::Focuser_::SetFocus(const PanePtr &pane) {
    ASSERT(pane);
    ASSERT(pane->GetInteractor());
    const PanePtr focused_pane = GetFocusedPane();
    if (pane != focused_pane) {
        auto it = std::find(panes_.begin(), panes_.end(), pane);
        ASSERTM(it != panes_.end(), desc_);
        ChangeFocus_(focused_pane, pane);
        focused_index_ = it - panes_.begin();
    }
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
        if (pane.IsEnabled() && pane.GetInteractor()->GetFocusBorder())
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

bool Panel::IsValid(std::string &details) {
    if (! SG::Node::IsValid(details))
        return false;

    if (! GetPane()) {
        details = "Missing Pane for " + GetDesc();
        return false;
    }

    return true;
}

void Panel::CreationDone() {
    SG::Node::CreationDone();

    if (! IsTemplate()) {
        focuser_.reset(new Focuser_(GetDesc()));

        // Add the root Pane as a child.
        ASSERTM(GetPane(), GetDesc());
        AddChild(GetPane());
    }
}

void Panel::SetContext(const ContextPtr &context) {
    ASSERT(! context_);  // Call only once.
    ASSERT(context);
    ASSERT(context->command_manager);
    ASSERT(context->name_manager);
    ASSERT(context->selection_manager);
    ASSERT(context->session_manager);
    ASSERT(context->settings_manager);
    ASSERT(context->panel_helper);

    context_ = context;

    // The context is required for UpdateInteractivePanes_() to work, so do it
    // now.
    UpdateInteractivePanes_();
}

void Panel::SetTestContext(const ContextPtr &context) {
    ASSERT(context);
    context_ = context;

    // Same as in SetContext().
    UpdateInteractivePanes_();
}

void Panel::SetSize(const Vector2f &size) {
    // Can set the size only if it has not been set or the Panel is resizable.
    ASSERT(GetSize() == Vector2f::Zero() || IsResizable());
    if (auto pane = GetPane())
        pane->SetLayoutSize(size);
}

Vector2f Panel::GetSize() const {
    if (auto pane = GetPane())
        return pane->GetLayoutSize();
    return Vector2f::Zero();
}

Vector2f Panel::UpdateSize() {
    auto pane = GetPane();
    ASSERT(pane);

    // Do this until the base size is not changing any more.
    Vector2f size;
    while (true) {
        // Make sure the layout size is at least as large as the base
        // size. This also makes sure the base sizes in all Panes are up to
        // date.
        size = MaxComponents(pane->GetBaseSize(), pane->GetLayoutSize());
        KLOG('p', GetDesc() << " updating size to " << size);

        // Lay out the Panes again to make sure they are the correct size.
        pane->SetLayoutSize(size);

        // Stop if there are no more base size changes.
        if (pane->IsBaseSizeUpToDate())
            break;
    }

    // Let the derived class know the Pane size may have changed.
    UpdateForPaneSizeChange();

    size_may_have_changed_ = false;

    return size;
}

Vector2f Panel::GetMinSize() const {
    if (auto pane = GetPane())
        return pane->GetBaseSize();
    return Vector2f::Zero();
}

bool Panel::HandleEvent(const Event &event) {
    // If there is a VirtualKeyboard, track headset on/off events to set its
    // visibility.
    if (GetContext().virtual_keyboard &&
        (event.flags.Has(Event::Flag::kButtonPress) ||
         event.flags.Has(Event::Flag::kButtonRelease)) &&
        event.button == Event::Button::kHeadset)
        GetContext().virtual_keyboard->SetIsVisible(
            event.flags.Has(Event::Flag::kButtonPress));

    bool handled = false;

    // Let the focused Pane (if any) handle the event.
    if (auto focused_pane = focuser_->GetFocusedPane()) {
        ASSERT(focused_pane->GetInteractor());
        handled = focused_pane->GetInteractor()->HandleEvent(event);
        if (handled) {
            KLOG('h', focused_pane->GetName() << " in "
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

void Panel::SetIsShown(bool is_shown) {
    if (is_shown) {
        // Let the derived class update any UI.
        UpdateInterface();
        focuser_->InitFocus();
    }
}

WidgetPtr Panel::GetIntersectedPaneWidget(const Point3f &pos, float radius,
                                          const Matrix4f &panel_to_world) {
    // This intersection function is used by Panes to see if the touch sphere
    // intersects a Node (typically a Widget). It converts the Node's bounds
    // into world coordinates, which is where the touch sphere is defined.
    const Pane::IntersectionFunc intersect_func = [&](const SG::Node &node,
                                                      float &dist){
        const SG::CoordConv cc = GetCoordConv_(node);
        const Matrix4f p2w = panel_to_world * cc.GetObjectToRootMatrix();
        const auto bounds = TransformBounds(node.GetBounds(), p2w);
        return SphereBoundsIntersect(pos, radius, bounds, dist);
    };

    // Recurse on all Panes.
    float closest_dist = std::numeric_limits<float>::max();
    return GetPane()->GetIntersectedWidget(intersect_func, closest_dist);
}

void Panel::PostSetUpIon() {
    SG::Node::PostSetUpIon();

    // Let the derived class set up.
    InitInterface();

    // Detect root Pane base size changes.
    auto &root_pane = GetPane();
    root_pane->GetBaseSizeChanged().AddObserver(
        this, [&](){ size_may_have_changed_ = true; });

    // Update when root Pane contents change.
    root_pane->GetContentsChanged().AddObserver(
        this, [&](){ ProcessPaneContentsChange_(); });
}

Panel::Context & Panel::GetContext() const {
    ASSERT(context_);
    return *context_;
}

void Panel::ResetSize() {
    if (auto pane = GetPane())
        pane->ResetLayoutSize();
}

const Settings & Panel::GetSettings() const {
    return GetContext().settings_manager->GetSettings();
}

void Panel::AddButtonFunc(const std::string &name, const ButtonFunc &func) {
    auto but_pane = GetPane()->FindTypedPane<ButtonPane>(name);
    auto &clicked = but_pane->GetButton().GetClicked();
    clicked.AddObserver(this, [func](const ClickInfo &){ func(); });
}

void Panel::SetButtonText(const std::string &name, const std::string &text) {
    auto but_pane  = GetPane()->FindTypedPane<ButtonPane>(name);
    auto text_pane = but_pane->FindTypedPane<TextPane>("ButtonText");
    text_pane->SetText(text);
}

void Panel::EnableButton(const std::string &name, bool enabled) {
    auto but_pane = GetPane()->FindTypedPane<ButtonPane>(name);
    but_pane->SetInteractionEnabled(enabled);
}

void Panel::SetFocus(const PanePtr &pane) {
    focuser_->SetFocus(pane);
}

void Panel::SetFocus(const std::string &name) {
    SetFocus(GetPane()->FindPane(name));
}

PanePtr Panel::GetFocusedPane() const {
    return focuser_->GetFocusedPane();
}

void Panel::DisplayMessage(const std::string &message,
                           const MessageFunc &func) {
    auto &helper = *GetContext().panel_helper;
    auto dp = helper.GetTypedPanel<DialogPanel>("DialogPanel");
    dp->SetMessage(message);
    dp->SetSingleResponse("OK");

    auto result_func = [func](const std::string &){
        if (func)
            func();
    };
    helper.PushPanel(dp, result_func);
}

void Panel::AskQuestion(const std::string &question, const QuestionFunc &func,
                        bool is_no_default) {
    ASSERT(func);

    auto &helper = *GetContext().panel_helper;
    auto dp = helper.GetTypedPanel<DialogPanel>("DialogPanel");
    dp->SetMessage(question);
    dp->SetChoiceResponse("No", "Yes", is_no_default);

    helper.PushPanel(dp, func);
}

void Panel::Close(const std::string &result) {
    context_->panel_helper->ClosePanel(result);
}

void Panel::UpdateInteractivePanes_() {
    if (auto &vk = GetContext().virtual_keyboard)
        focuser_->SetVirtualKeyboard(vk);
    std::vector<PanePtr> interactive_panes;
    FindInteractivePanes_(GetPane(), interactive_panes);
    focuser_->SetPanes(interactive_panes);
}

void Panel::FindInteractivePanes_(const PanePtr &pane,
                                  std::vector<PanePtr> &panes) {
    if (pane->GetInteractor())
        panes.push_back(pane);

    // Recurse if the Pane is a ContainerPane.
    if (ContainerPanePtr ctr = Util::CastToDerived<ContainerPane>(pane)) {
        for (auto &sub_pane: ctr->GetPotentialInteractiveSubPanes())
            FindInteractivePanes_(sub_pane, panes);
    }
}

bool Panel::ProcessKeyPress_(const Event &event) {
    const std::string key_string = event.GetKeyString();
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
    else if (key_string == "Tab" || key_string == "<Shift>Tab") {
        focuser_->MoveFocus(key_string == "Tab" ? Focuser_::Direction::kDown :
                            Focuser_::Direction::kUp);
        handled = true;
        KLOG('h', GetName() << " handled navigation key press");
    }

    return handled;
}

void Panel::ProcessPaneContentsChange_() {
    UpdateInteractivePanes_();
}

SG::CoordConv Panel::GetCoordConv_(const SG::Node &node) {
    // Create an std::shared_ptr that does not delete this.
    auto np = Util::CreateTemporarySharedPtr<SG::Node>(this);
    const SG::NodePath path = SG::FindNodePathUnderNode(np, node);
    return SG::CoordConv(path);
}
