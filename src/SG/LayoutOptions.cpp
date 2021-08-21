#include "SG/LayoutOptions.h"

#include "SG/SpecBuilder.h"

namespace SG {

void LayoutOptions::SetUpIon(IonContext &context) {
    // Always do this, since it is fast.
    ion_layout_options_.target_point            = target_point_;
    ion_layout_options_.target_size             = target_size_;
    ion_layout_options_.horizontal_alignment    = halignment_;
    ion_layout_options_.vertical_alignment      = valignment_;
    ion_layout_options_.line_spacing            = line_spacing_;
    ion_layout_options_.glyph_spacing           = glyph_spacing_;
    ion_layout_options_.metrics_based_alignment = use_metrics_;
}

Parser::ObjectSpec LayoutOptions::GetObjectSpec() {
    SG::SpecBuilder<LayoutOptions> builder;
    builder.AddPoint2f("target_point",        &LayoutOptions::target_point_);
    builder.AddVector2f("target_size",        &LayoutOptions::target_size_);
    builder.AddEnum<HAlignment>("halignment", &LayoutOptions::halignment_);
    builder.AddEnum<VAlignment>("valignment", &LayoutOptions::valignment_);
    builder.AddFloat("line_spacing",          &LayoutOptions::line_spacing_);
    builder.AddFloat("glyph_spacing",         &LayoutOptions::glyph_spacing_);
    builder.AddBool("use_metrics",            &LayoutOptions::use_metrics_);
    return Parser::ObjectSpec{
        "LayoutOptions", false, []{ return new LayoutOptions; },
        builder.GetSpecs() };
}

}  // namespace SG
