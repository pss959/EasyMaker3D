#include "Panels/Panel.h"

#include "CoordConv.h"
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
        // Add the root Pane as a child.
        AddChild(GetPane());

        // Find the highlight PolyLine.
        if (! highlight_line_) {
            auto node = SG::FindNodeUnderNode(*this, "FocusHighlight");
            ASSERT(node->GetShapes().size() == 1U);
            highlight_line_ =
                Util::CastToDerived<SG::PolyLine>(node->GetShapes()[0]);
            ASSERT(highlight_line_);
        }
    }
}

void Panel::SetContext(const ContextPtr &context) {
    ASSERT(context);
    ASSERT(context->session_manager);
    ASSERT(context->settings_manager);
    ASSERT(context->selection_manager);
    ASSERT(context->panel_helper);

    context_ = context;
}

void Panel::SetSize(const Vector2f &size) {
    if (auto pane = GetPane())
        pane->SetSize(size);
    if (focused_index_ >= 0)
        HighlightFocusedPane_();
}

Vector2f Panel::GetSize() const {
    if (auto pane = GetPane())
        return pane->GetSize();
    return Vector2f::Zero();
}

Vector2f Panel::GetBaseSize() const {
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
            ChangeFocus_(1);
            handled = true;
        }
        else if (key_string == "<Shift>Tab") {
            ChangeFocus_(-1);
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

    // Pass root Pane size changes to observers.
    GetPane()->GetSizeChanged().AddObserver(
        this, [&](const Pane &){ ProcessSizeChange_(); });
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

void Panel::SetFocus(const std::string &name) {
    auto pane = GetPane()->FindPane(name);
    ASSERT(pane->IsInteractive());
    auto it = std::find(interactive_panes_.begin(),
                        interactive_panes_.end(), pane);
    ASSERT(it != interactive_panes_.end());
    focused_index_ = it - interactive_panes_.begin();
    if (highlight_line_)
        HighlightFocusedPane_();
}

PanePtr Panel::GetFocusedPane() const {
    if (focused_index_ >= 0)
        return interactive_panes_[focused_index_];
    else
        return PanePtr();
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
    std::vector<Point3f> pts(5);
    pts[0].Set(min_p[0], min_p[1], max_p[2]);
    pts[1].Set(max_p[0], min_p[1], max_p[2]);
    pts[2].Set(max_p[0], max_p[1], max_p[2]);
    pts[3].Set(min_p[0], max_p[1], max_p[2]);
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

void Panel::ProcessSizeChange_() {
    // Update the focus highlight in case a relevant Pane changed.
    if (focused_index_ >= 0)
        HighlightFocusedPane_();

    // Notify observers.
    size_changed_.Notify();
}

void Panel::ChangeFocus_(int increment) {
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

    if (new_index != focused_index_) {
        interactive_panes_[focused_index_]->Deactivate();
        focused_index_ = new_index;
        HighlightFocusedPane_();
    }
}
