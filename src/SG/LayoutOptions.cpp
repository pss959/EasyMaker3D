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

}  // namespace SG
