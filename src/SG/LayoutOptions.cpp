#include "SG/LayoutOptions.h"


namespace SG {

void LayoutOptions::AddFields() {
    AddField(halignment_);
    AddField(valignment_);
    AddField(line_spacing_);
    AddField(glyph_spacing_);
    AddField(use_metrics_);
    Object::AddFields();
}

void LayoutOptions::SetHAlignment(HAlignment alignment) {
    halignment_ = alignment;
    ProcessChange(Change::kGeometry, *this);
}

void LayoutOptions::SetVAlignment(VAlignment alignment) {
    valignment_ = alignment;
    ProcessChange(Change::kGeometry, *this);
}

void LayoutOptions::SetLineSpacing(float spacing) {
    line_spacing_ = spacing;
    ProcessChange(Change::kGeometry, *this);
}

void LayoutOptions::SetGlyphSpacing(float spacing) {
    glyph_spacing_ = spacing;
    ProcessChange(Change::kGeometry, *this);
}

}  // namespace SG
