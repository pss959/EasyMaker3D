#include "Panels/Panel.h"

#include <limits>

#include <ion/math/transformutils.h>

#include "App/ClickInfo.h"
#include "Base/Tuning.h"
#include "Base/VirtualKeyboard.h"
#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Managers/SessionManager.h"
#include "Managers/SettingsManager.h"
#include "Math/Intersection.h"
#include "Math/Linear.h"
#include "Panels/DialogPanel.h"
#include "Panes/ButtonPane.h"
#include "Panes/TextPane.h"
#include "SG/PolyLine.h"
#include "SG/Search.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Widgets/ClickableWidget.h"
#include "Widgets/PushButtonWidget.h"

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
        ASSERTM(GetPane(), GetDesc());

        // Add the root Pane as a child.
        AddChild(GetPane());

        // Find the highlight PolyLine.
        if (! highlight_line_) {
            auto node = SG::FindNodeUnderNode(*this, "FocusHighlight");
            highlight_line_ =
                SG::FindTypedShapeInNode<SG::PolyLine>(*node, "Lines");
        }
    }
}

void Panel::SetContext(const ContextPtr &context) {
    ASSERT(! context_);  // Call only once.
    ASSERT(context);
    ASSERT(context->name_manager);
    ASSERT(context->selection_manager);
    ASSERT(context->session_manager);
    ASSERT(context->settings_manager);
    ASSERT(context->panel_helper);

    context_ = context;

    // The context is required for FindInteractivePanes_() to work, so do it
    // now.
    ASSERT(interactive_panes_.empty());
    FindInteractivePanes_(GetPane());
}

void Panel::SetTestContext(const ContextPtr &context) {
    ASSERT(context);
    context_ = context;

    // Same as in SetContext().
    ASSERT(interactive_panes_.empty());
    FindInteractivePanes_(GetPane());
}

void Panel::SetSize(const Vector2f &size) {
    // Can set the size only if it has not been set or the Panel is resizable.
    ASSERT(GetSize() == Vector2f::Zero() || IsResizable());
    if (auto pane = GetPane())
        pane->SetLayoutSize(size);
    if (focused_index_ >= 0)
        update_focus_highlight_ = true;
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

    // The focused Pane may have changed size.
    if (focused_index_ >= 0)
        update_focus_highlight_ = true;

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
    if (auto focused_pane = GetFocusedPane()) {
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

    if (handled)
        update_focus_highlight_ = true;

    // Handle all valuator events so the MainHandler does not get them. If the
    // focused Pane is a ScrollingPane, pass the event to it.
    if (! handled && event.flags.Has(Event::Flag::kPosition1D))
        handled = true;

    return handled;
}

void Panel::SetIsShown(bool is_shown) {
    if (is_shown) {
        // Let the derived class update any UI.
        UpdateInterface();

        if (! interactive_panes_.empty()) {
            // Use first interactive pane by default.
            if (focused_index_ < 0) {
                focused_index_ = 0;
                KLOG('F', GetDesc() << " focused on "
                     << interactive_panes_[0]->GetDesc() << " to start");
            }
            update_focus_highlight_ = true;
        }
    }
}

WidgetPtr Panel::GetIntersectedPaneWidget(const Point3f &pos, float radius,
                                          const Matrix4f &panel_to_world) {
    // This intersection function is used by Panes to see if the touch sphere
    // intersects a Node (typically a Widget). It converts the Node's bounds
    // into world coordinates, which is where the touch sphere is defined.
    const Pane::IntersectionFunc intersect_func = [&](const SG::Node &node,
                                                      float &dist){
        const CoordConv cc = GetCoordConv_(node);
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

void Panel::UpdateForRenderPass(const std::string &pass_name) {
    SG::Node::UpdateForRenderPass(pass_name);
    if (update_focus_highlight_) {
        if (focused_index_ >= 0)
            HighlightFocusedPane_();
        update_focus_highlight_ = false;
    }
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
    ASSERT(pane);
    ASSERT(pane->GetInteractor());
    auto it = std::find(interactive_panes_.begin(),
                        interactive_panes_.end(), pane);
    ASSERTM(it != interactive_panes_.end(), pane->GetDesc());
    const size_t index = it - interactive_panes_.begin();
    if (static_cast<int>(index) != focused_index_)
        ChangeFocusTo_(index);
}

void Panel::SetFocus(const std::string &name) {
    SetFocus(GetPane()->FindPane(name));
}

PanePtr Panel::GetFocusedPane() const {
    if (focused_index_ >= 0)
        return interactive_panes_[focused_index_];
    else
        return PanePtr();
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

void Panel::AskQuestion(const std::string &question, const QuestionFunc &func) {
    ASSERT(func);

    auto &helper = *GetContext().panel_helper;
    auto dp = helper.GetTypedPanel<DialogPanel>("DialogPanel");
    dp->SetMessage(question);
    dp->SetChoiceResponse("No", "Yes");

    helper.PushPanel(dp, func);
}

void Panel::Close(const std::string &result) {
    context_->panel_helper->ClosePanel(result);
}

void Panel::FindInteractivePanes_(const PanePtr &pane) {
    if (pane->GetInteractor()) {
        interactive_panes_.push_back(pane);
        InitPaneInteraction_(pane);
    }
    if (ContainerPanePtr ctr = Util::CastToDerived<ContainerPane>(pane)) {
        for (auto &sub_pane: ctr->GetPotentialInteractiveSubPanes())
            FindInteractivePanes_(sub_pane);
    }
}

void Panel::InitPaneInteraction_(const PanePtr &pane) {
    ASSERT(pane->GetInteractor());
    auto &interactor = *pane->GetInteractor();

    // Tell the IPaneInteractor about the VirtualKeyboard, if any.
    if (auto &vk = GetContext().virtual_keyboard)
        interactor.SetVirtualKeyboard(vk);

    // If there is an activator Widget, observe its GetActivation() Notifier if
    // not already done. This is better than using the GetClicked() Notifier
    // because the observer should be notified at the start of a click or drag.
    if (auto clickable = interactor.GetActivationWidget()) {
        if (! clickable->GetActivation().HasObserver(this)) {
            auto func = [&, pane](Widget &, bool is_act){
                if (is_act) {
                    SetFocus(pane);
                    ActivatePane_(pane, true);
                }
            };
            clickable->GetActivation().AddObserver(this, func);
        }
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
                ActivatePane_(focused_pane, false);
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
        ChangeFocusBy_(key_string == "Tab" ? 1 : -1);
        handled = true;
        KLOG('h', GetName() << " handled navigation key press");
    }

    return handled;
}

void Panel::HighlightFocusedPane_() {
    ASSERT(highlight_line_);

    ASSERT(static_cast<size_t>(focused_index_) <= interactive_panes_.size());
    auto &pane = interactive_panes_[focused_index_];

    // Set the points of the PolyLine in object coordinates of the Pane.
    const Bounds bounds = pane->GetBounds();
    const Point3f min_p = bounds.GetMinPoint();
    const Point3f max_p = bounds.GetMaxPoint();
    // Move forward in Z a little.
    const float z = max_p[2] + TK::kPaneZOffset;

    std::vector<Point3f> pts(5);
    pts[0].Set(min_p[0], min_p[1], z);
    pts[1].Set(max_p[0], min_p[1], z);
    pts[2].Set(max_p[0], max_p[1], z);
    pts[3].Set(min_p[0], max_p[1], z);
    pts[4] = pts[0];

    // Convert coordinates from the focused pane to the local coordinates of
    // the Panel.
    const CoordConv cc = GetCoordConv_(*pane);
    for (auto &p: pts)
        p = cc.ObjectToRoot(p);

    highlight_line_->SetPoints(pts);
}

void Panel::ProcessPaneContentsChange_() {
    // Update the interactive panes.
    interactive_panes_.clear();
    FindInteractivePanes_(GetPane());
    update_focus_highlight_ = true;
}

void Panel::ChangeFocusBy_(int increment) {
    // Has to be a starting point.
    if (focused_index_ < 0)
        return;

    // Keep going in the specified direction until an enabled interactive pane
    // is found or the original focused pane is hit again.
    int new_index = focused_index_;
    while (true) {
        new_index = new_index + increment;
        if (new_index < 0)
            new_index = interactive_panes_.size() - 1;
        else if (static_cast<size_t>(new_index) >= interactive_panes_.size())
            new_index = 0;
        if (new_index == focused_index_)
            break;  // No other interactive pane.
        auto &pane = *interactive_panes_[new_index];
        if (pane.IsEnabled() && pane.GetInteractor()->CanFocus())
            break;
    }

    if (new_index != focused_index_)
        ChangeFocusTo_(new_index);
}

void Panel::ChangeFocusTo_(size_t index) {
    ASSERT(index < interactive_panes_.size());
    ASSERT(static_cast<int>(index) != focused_index_);
    if (focused_index_ >= 0) {
        auto &pane = *interactive_panes_[focused_index_];
        KLOG('F', GetDesc() << " removing focus from " << pane.GetDesc());
        auto &interactor = *pane.GetInteractor();
        if (interactor.IsActive())
            interactor.Deactivate();
        interactor.SetFocus(false);
    }
    focused_index_ = index;
    if (focused_index_ >= 0) {
        const auto &pane = interactive_panes_[focused_index_];
        KLOG('F', GetDesc() << " adding focus to " << pane->GetDesc());
        pane->GetInteractor()->SetFocus(true);
        UpdateFocus(pane);
    }
    update_focus_highlight_ = true;
}

void Panel::ActivatePane_(const PanePtr &pane, bool is_click) {
    ASSERT(pane->GetInteractor());
    auto &interactor = *pane->GetInteractor();

    // Make sure the Pane is still able to interact. It may have been disabled
    // by another observer.
    if (! interactor.CanFocus()) {
        ChangeFocusBy_(1);
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

    KLOG('F', GetDesc() << " activating " << pane->GetDesc());
    interactor.Activate();
}

CoordConv Panel::GetCoordConv_(const SG::Node &node) {
    // Create an std::shared_ptr that does not delete this.
    auto np = Util::CreateTemporarySharedPtr<SG::Node>(this);
    const SG::NodePath path = SG::FindNodePathUnderNode(np, node);
    return CoordConv(path);
}
