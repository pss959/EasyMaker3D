#include "Panels/Panel.h"

#include "CoordConv.h"
#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Managers/SessionManager.h"
#include "Managers/SettingsManager.h"
#include "Math/Linear.h"
#include "Panels/DialogPanel.h"
#include "Panes/ButtonPane.h"
#include "Panes/TextPane.h"
#include "SG/PolyLine.h"
#include "SG/Search.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Widgets/PushButtonWidget.h"

void Panel::AddFields() {
    AddField(pane_);
    AddField(is_movable_);
    AddField(is_resizable_);
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
    ASSERT(context);
    ASSERT(context->name_manager);
    ASSERT(context->selection_manager);
    ASSERT(context->session_manager);
    ASSERT(context->settings_manager);
    ASSERT(context->panel_helper);

    context_ = context;
}

void Panel::SetTestContext(const ContextPtr &context) {
    ASSERT(context);
    context_ = context;
}

void Panel::SetSize(const Vector2f &size) {
    // Can set the size only if it has not been set or the Panel is resizable.
    ASSERT(GetSize() == Vector2f::Zero() || IsResizable());
    if (auto pane = GetPane())
        pane->SetLayoutSize(size);
    if (focused_index_ >= 0)
        HighlightFocusedPane_();
}

Vector2f Panel::GetSize() const {
    if (auto pane = GetPane())
        return pane->GetLayoutSize();
    return Vector2f::Zero();
}

void Panel::UpdateSize() {
    // Calling this will cause the ContainerPane to lay out again if anything
    // has changed.
    if (auto pane = GetPane()) {
        // Make sure the layout size is at least as large as the base size.
        const Vector2f size = MaxComponents(pane->GetBaseSize(),
                                            pane->GetLayoutSize());
        pane->SetLayoutSize(size);
    }
}

Vector2f Panel::GetMinSize() const {
    if (auto pane = GetPane())
        return pane->GetBaseSize();
    return Vector2f::Zero();
}

bool Panel::HandleEvent(const Event &event) {
    bool handled = false;
    if (event.flags.Has(Event::Flag::kKeyPress)) {
        const std::string key_string = event.GetKeyString();
        if (key_string == "Escape") {
            Close("Cancel");
            handled = true;
        }
        else if (key_string == "Tab") {
            ChangeFocusBy_(1);
            handled = true;
        }
        else if (key_string == "<Shift>Tab") {
            ChangeFocusBy_(-1);
            handled = true;
        }
        else if (key_string == "Enter" && focused_index_ >= 0) {
            interactive_panes_[focused_index_]->Activate();
            handled = true;
        }
        // Otherwise ask the focused pane, if any.
        else if (focused_index_ >= 0) {
            handled = interactive_panes_[focused_index_]->HandleEvent(event);
        }
    }

    return handled;
}

void Panel::SetIsShown(bool is_shown) {
    if (is_shown) {
        // Let the derived class update any UI.
        UpdateInterface();

        if (! interactive_panes_.empty()) {
            // Use first interactive pane by default.
            if (focused_index_ < 0)
                focused_index_ = 0;
            HighlightFocusedPane_();
        }
    }
}

void Panel::PostSetUpIon() {
    SG::Node::PostSetUpIon();

    // Let the derived class set up.
    InitInterface();

    ASSERT(interactive_panes_.empty());
    FindInteractivePanes_(GetPane());
    SetUpButtons_();

    // Tell each interactive Pane how to get focus. This is needed when a Pane
    // detects a click on itself.
    auto focus_func = [&](const Pane &pane){ SetFocus(pane.GetName()); };
    for (auto &pane: interactive_panes_)
        pane->SetFocusFunc(focus_func);

    // Pass root Pane base size changes to observers.
    auto &root_pane = GetPane();
    root_pane->GetBaseSizeChanged().AddObserver(
        this, [&](){ size_changed_.Notify();});

    // Update when root Pane contents change.
    root_pane->GetContentsChanged().AddObserver(
        this, [&](){ ProcessPaneContentsChange_(); });
}

Panel::Context & Panel::GetContext() const {
    ASSERT(context_);
    return *context_;
}

void Panel::AddButtonFunc(const std::string &name, const ButtonFunc &func) {
    ASSERT(! Util::MapContains(button_func_map_, name));
    button_func_map_[name] = func;
}

const Settings & Panel::GetSettings() const {
    return GetContext().settings_manager->GetSettings();
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
    ASSERT(pane->IsInteractive());
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
    auto init = [&](const PanelPtr &p){
        ASSERT(p->GetTypeName() == "DialogPanel");
        DialogPanel &dp = *Util::CastToDerived<DialogPanel>(p);
        dp.SetMessage(message);
        dp.SetSingleResponse("OK");
    };

    // Save the function so it is around when the DialogPanel finishes.
    message_func_ = func;

    auto result = [&](Panel &, const std::string &){
        if (message_func_) {
            message_func_();
            message_func_ = nullptr;
        }
    };
    GetContext().panel_helper->Replace("DialogPanel", init, result);
}

void Panel::AskQuestion(const std::string &question, const QuestionFunc &func) {
    ASSERT(func);
    auto init = [&](const PanelPtr &p){
        ASSERT(p->GetTypeName() == "DialogPanel");
        DialogPanel &dp = *Util::CastToDerived<DialogPanel>(p);
        dp.SetMessage(question);
        dp.SetChoiceResponse("No", "Yes");
    };

    // Save the function so it is around when the DialogPanel finishes.
    question_func_ = func;

    auto result = [&](Panel &, const std::string &res){
        question_func_(res);
        question_func_ = nullptr;
    };
    GetContext().panel_helper->Replace("DialogPanel", init, result);
}

void Panel::FindInteractivePanes_(const PanePtr &pane) {
    if (pane->IsEnabled() && pane->IsInteractive())
        interactive_panes_.push_back(pane);
    if (ContainerPanePtr ctr = Util::CastToDerived<ContainerPane>(pane)) {
        for (auto &sub_pane: ctr->GetPanes())
            FindInteractivePanes_(sub_pane);
    }
}

void Panel::SetUpButtons_() {
    auto is_button = [](const Node &node){
        return dynamic_cast<const ButtonPane *>(&node);
    };
    for (auto &but_node: SG::FindNodes(GetPane(), is_button)) {
        ButtonPanePtr but = Util::CastToDerived<ButtonPane>(but_node);
        ASSERT(but);

        // If there are no observers for the button, add one.
        auto &clicked = but->GetButton().GetClicked();
        if (! clicked.GetObserverCount()) {
            clicked.AddObserver(
                this, [this, but](const ClickInfo &){
                    const std::string &name = but->GetName();
                    ASSERTM(Util::MapContains(button_func_map_, name),
                            "No function specified for button " + name);
                    button_func_map_[name]();
                });
        }
    }
}

void Panel::HighlightFocusedPane_() {
    ASSERT(highlight_line_);

    if (focused_index_ < 0) {
        KLOG('F', GetDesc() << " has no focused Pane");
        return;
    }

    ASSERT(static_cast<size_t>(focused_index_) <= interactive_panes_.size());
    auto &pane = interactive_panes_[focused_index_];
    KLOG('F', GetDesc() << " focus on " << pane->ToString());

    // Set the points of the PolyLine in object coordinates of the Pane.
    const Bounds bounds = pane->GetBounds();
    const Point3f min_p = bounds.GetMinPoint();
    const Point3f max_p = bounds.GetMaxPoint();
    // Move forward in Z a little.
    const float z = max_p[2] + .01f;

    std::vector<Point3f> pts(5);
    pts[0].Set(min_p[0], min_p[1], z);
    pts[1].Set(max_p[0], min_p[1], z);
    pts[2].Set(max_p[0], max_p[1], z);
    pts[3].Set(min_p[0], max_p[1], z);
    pts[4] = pts[0];

    // Creates an std::shared_ptr that does not delete this.
    auto np = Util::CreateTemporarySharedPtr<SG::Node>(this);

    // Find the path from this Panel to the focused Pane and convert coordinates
    // to the local coordinates of the Panel (which are equivalent to the
    // "world" coordinates of the path).
    const SG::NodePath path = SG::FindNodePathUnderNode(np, *pane);
    ASSERT(! path.empty());
    for (auto &p: pts)
        p = CoordConv(path).ObjectToRoot(p);

    highlight_line_->SetPoints(pts);
}

void Panel::ProcessPaneContentsChange_() {
    // Update the interactive panes.
    interactive_panes_.clear();
    FindInteractivePanes_(GetPane());

    // Update the focus highlight in case a relevant Pane changed.
    if (focused_index_ >= 0)
        HighlightFocusedPane_();
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
        if (interactive_panes_[new_index]->IsInteractionEnabled())
            break;
    }

    if (new_index != focused_index_)
        ChangeFocusTo_(new_index);
}

void Panel::ChangeFocusTo_(size_t index) {
    ASSERT(index < interactive_panes_.size());
    ASSERT(static_cast<int>(index) != focused_index_);
    if (focused_index_ >= 0)
        interactive_panes_[focused_index_]->Deactivate();
    focused_index_ = index;
    if (highlight_line_)
        HighlightFocusedPane_();
}
