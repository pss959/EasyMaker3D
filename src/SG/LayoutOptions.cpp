#include "SG/LayoutOptions.h"


namespace SG {

void LayoutOptions::AddFields() {
    AddField(target_point_);
    AddField(target_size_);
    AddField(halignment_);
    AddField(valignment_);
    AddField(line_spacing_);
    AddField(glyph_spacing_);
    AddField(use_metrics_);
    Object::AddFields();
}

void LayoutOptions::SetTargetSize(const Vector2f &size) {
    target_size_ = size;
    ProcessChange(Change::kGeometry);
}

void LayoutOptions::SetHAlignment(HAlignment alignment) {
    halignment_ = alignment;
    ProcessChange(Change::kGeometry);
}

void LayoutOptions::SetVAlignment(VAlignment alignment) {
    valignment_ = alignment;
    ProcessChange(Change::kGeometry);
}

}  // namespace SG
