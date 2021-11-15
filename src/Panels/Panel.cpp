#include "Panels/Panel.h"

#include "Panes/ButtonPane.h"
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

void Panel::SetSize(const Vector2f &size) {
    if (auto pane = GetPane())
        pane->SetSize(size);
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
    }

    return false;
}

void Panel::PreSetUpIon() {
    SG::Node::PreSetUpIon();

    // Add the root Pane as a child.
    AddChild(GetPane());
}

void Panel::PostSetUpIon() {
    SG::Node::PostSetUpIon();

    FindInteractivePanes_();
    SetUpButtons_();
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

void Panel::Close(const std::string &result) {
    if (closed_func_)
        closed_func_(result);
}
