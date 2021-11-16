#include "Panels/Panel.h"

#include "Panes/ButtonPane.h"
#include "SG/PolyLine.h"
#include "SG/Search.h"
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

void Panel::SetContext(const ContextPtr &context) {
    ASSERT(context);
    // ASSERT(context->XXXX);

    context_ = context;
}

void Panel::SetSize(const Vector2f &size) {
    if (auto pane = GetPane())
        pane->SetSize(size);
    if (focused_index_ >= 0)
        HighlightFocusedPane_();
}

Vector2f Panel::GetMinSize() const {
    Vector2f min_size(0, 0);
    if (auto pane = GetPane())
        min_size = pane->GetMinSize();
    return min_size;
}

bool Panel::HandleEvent(const Event &event) {
    // Handle escape key to cancel.
    if (event.flags.Has(Event::Flag::kKeyPress)) {
        if (event.key_string == "Escape") {
            Close("Cancel");
        }
        else if (event.key_string == "Tab") {
            ChangeFocus_(1);
        }
        else if (event.key_string == "<Shift>Tab") {
            ChangeFocus_(-1);
        }
        else if (event.key_string == "Enter" && focused_index_ >= 0) {
            interactive_panes_[focused_index_]->Activate();
        }
    }

    return false;
}

void Panel::SetIsShown(bool is_shown) {
    if (is_shown) {
        // Let the derived class update any UI.
        UpdateInterface();

        if (! interactive_panes_.empty()) {
            if (focused_index_ < 0)
                focused_index_ = 0;  // XXXX
            HighlightFocusedPane_();
        }
    }
}

void Panel::PreSetUpIon() {
    SG::Node::PreSetUpIon();

    // Add the root Pane as a child.
    AddChild(GetPane());

    // Fin the highlight PolyLine.
    if (! highlight_line_) {
        auto node = SG::FindNodeUnderNode(*this, "FocusHighlight");
        ASSERT(node->GetShapes().size() == 1U);
        highlight_line_ =
            Util::CastToDerived<SG::PolyLine>(node->GetShapes()[0]);
        ASSERT(highlight_line_);
    }
}

void Panel::PostSetUpIon() {
    SG::Node::PostSetUpIon();

    FindInteractivePanes_();
    SetUpButtons_();

    // Set up this Panel as an observer of size changes to any Pane.
    GetPane()->GetPaneSizeChanged().AddObserver(
        this, std::bind(&Panel::ProcessPaneSizeChange_, this));
}

Panel::Context & Panel::GetContext() const {
    ASSERT(context_);
    return *context_;
}

void Panel::Close(const std::string &result) {
    if (closed_func_)
        closed_func_(result);
}

void Panel::FindInteractivePanes_() {
    auto is_interactive_pane = [](const Node &node){
        auto pane = dynamic_cast<const Pane *>(&node);
        return pane && pane->IsInteractive();
    };
    for (auto &node: SG::FindNodes(GetPane(), is_interactive_pane)) {
        interactive_panes_.push_back(Util::CastToDerived<Pane>(node));
        ASSERT(interactive_panes_.back());
    }
}

void Panel::SetUpButtons_() {
    auto is_button = [](const Node &node){
        return dynamic_cast<const ButtonPane *>(&node);
    };
    for (auto &but_node: SG::FindNodes(GetPane(), is_button)) {
        ButtonPanePtr but = Util::CastToDerived<ButtonPane>(but_node);
        ASSERT(but);
        but->GetButton().GetClicked().AddObserver(
            this, [this, but](const ClickInfo &){
                ProcessButton(but->GetName());
            });
    }
}

void Panel::HighlightFocusedPane_() {
    ASSERT(highlight_line_);
    ASSERT(focused_index_ >= 0 &&
           static_cast<size_t>(focused_index_) <= interactive_panes_.size());
    auto &pane = interactive_panes_[focused_index_];

    // Set the points of the PolyLine in local coordinates.
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
    auto np = SG::NodePtr(SG::NodePtr{}, this);

    // Find the path from this Panel to the Pane and convert coordinates to the
    // local coordinates of the Panel.
    const SG::NodePath path = SG::FindNodePathUnderNode(np, pane);
    ASSERT(! path.empty());
    for (auto &p: pts)
        p = path.FromLocal(p);

    highlight_line_->SetPoints(pts);
}

void Panel::ProcessPaneSizeChange_() {
    // Update the focus highlight in case a relevant Pane changed.
    if (focused_index_ >= 0)
        HighlightFocusedPane_();
}

void Panel::ChangeFocus_(int increment) {
    // Has to be a starting point.
    if (focused_index_ < 0)
        return;

    int new_index = focused_index_ + increment;
    if (new_index < 0)
        new_index = interactive_panes_.size() - 1;
    else if (static_cast<size_t>(new_index) >= interactive_panes_.size())
        new_index = 0;

    if (new_index != focused_index_) {
        focused_index_ = new_index;
        HighlightFocusedPane_();
    }
}
