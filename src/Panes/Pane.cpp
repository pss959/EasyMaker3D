#include "Panes/Pane.h"

void Pane::AddFields() {
    AddField(min_size_);
    AddField(resize_width_);
    AddField(resize_height_);
    AddField(color_);
    AddField(border_color_);
    AddField(border_width_);
    SG::Node::AddFields();
}

void Pane::SetSize(const Vector2f &size) {
    if (size_ != size) {
        size_ = size;
        ProcessChange(SG::Change::kGeometry);
    }
}
