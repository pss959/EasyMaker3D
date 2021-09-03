#include "SG/StateTable.h"

namespace SG {

void StateTable::AddFields() {
    AddField(clear_color_);
    AddField(clear_depth_);
    AddField(line_width_);
    AddField(depth_test_enabled_);
    AddField(cull_face_enabled_);
    AddField(cull_face_mode_);
}

void StateTable::SetUpIon(IonContext &context) {
    if (! ion_state_table_) {
        ion_state_table_.Reset(new ion::gfx::StateTable);

        // Change only the fields that were set. The Ion StateTable does not
        // send values for fields that are not modified.
        if (clear_color_.WasSet())
            ion_state_table_->SetClearColor(clear_color_);
        if (clear_depth_.WasSet())
            ion_state_table_->SetClearDepthValue(clear_depth_);
        if (line_width_.WasSet())
            ion_state_table_->SetLineWidth(line_width_);
        if (depth_test_enabled_.WasSet())
            ion_state_table_->Enable(Capability_::kDepthTest,
                                     depth_test_enabled_);
        if (cull_face_enabled_.WasSet())
            ion_state_table_->Enable(Capability_::kCullFace,
                                     cull_face_enabled_);
        if (cull_face_mode_.WasSet())
            ion_state_table_->SetCullFaceMode(cull_face_mode_);
    }
}

}  // namespace SG
