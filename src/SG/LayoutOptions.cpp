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
}

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

}  // namespace SG
