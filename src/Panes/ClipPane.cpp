#include "Panes/ClipPane.h"

#include "SG/Search.h"

void ClipPane::AddFields() {
    AddField(clip_size_);
    BoxPane::AddFields();
}

bool ClipPane::IsValid(std::string &details) {
    if (! BoxPane::IsValid(details))
        return false;
    if (GetClipSize()[0] <= 0 || GetClipSize()[1] <= 0) {
        details = "Non-positive clip size";
        return false;
    }
    return true;
}

void ClipPane::AllFieldsParsed(bool is_template) {
    BoxPane::AllFieldsParsed(is_template);
    pane_parent_ = SG::FindNodeUnderNode(*this, "PaneParent");
    clip_node_   = SG::FindNodeUnderNode(*this, "ClipNode");
}

void ClipPane::SetClipSize(const Vector2f &size) {
    clip_node_->SetScale(Vector3f(size[0], size[1], 1));
}
