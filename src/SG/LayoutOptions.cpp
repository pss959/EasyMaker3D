#include "SG/LayoutOptions.h"


namespace SG {

void LayoutOptions::AddFields() {
    AddField(halignment_.Init("halignment",       HAlignment::kAlignLeft));
    AddField(valignment_.Init("valignment",       VAlignment::kAlignBaseline));
    AddField(line_spacing_.Init("line_spacing",   1));
    AddField(glyph_spacing_.Init("glyph_spacing", 0));
    AddField(use_metrics_.Init("use_metrics",     false));

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
