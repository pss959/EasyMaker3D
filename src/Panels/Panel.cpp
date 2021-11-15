#include "Panels/Panel.h"

#include "Panes/ButtonPane.h"
#include "SG/Search.h"

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

    // Find and set up all buttons.
    auto &root_pane = GetPane();
    auto is_button = [](const Node &node){
        return dynamic_cast<const ButtonPane *>(&node);
    };
    for (auto &but_node: SG::FindNodes(root_pane, is_button)) {
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
