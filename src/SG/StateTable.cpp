#include "SG/StateTable.h"

namespace SG {

void StateTable::AddFields() {
    AddField(clear_color_);
    AddField(clear_depth_);
    AddField(clear_stencil_);
    AddField(line_width_);
    AddField(depth_test_enabled_);
    AddField(cull_face_enabled_);
    AddField(cull_face_mode_);
    AddField(create_stencil_);
    AddField(use_stencil_);
    Object::AddFields();
}

void StateTable::SetLineWidth(float width) {
    line_width_ = width;
    if (ion_state_table_)
        ion_state_table_->SetLineWidth(line_width_);
}

ion::gfx::StateTablePtr StateTable::SetUpIon() {
    ASSERT(! ion_state_table_);
    ion_state_table_.Reset(new ion::gfx::StateTable);

    // Change only the fields that were set. The Ion StateTable does not
    // send values for fields that are not modified.
    if (clear_color_.WasSet())
        ion_state_table_->SetClearColor(clear_color_);
    if (clear_depth_.WasSet())
        ion_state_table_->SetClearDepthValue(clear_depth_);
    if (clear_stencil_.WasSet())
        ion_state_table_->SetClearStencilValue(clear_stencil_);
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

    if (create_stencil_) {
        ion_state_table_->Enable(Capability_::kStencilTest, true);
        ion_state_table_->SetColorWriteMasks(false, false, false, false);
        ion_state_table_->SetDepthWriteMask(false);
        ion_state_table_->SetStencilWriteMasks(0xffffffff, 0xffffffff);
        ion_state_table_->SetStencilFunctions(
            ion::gfx::StateTable::kStencilAlways, 1, 0xff,
            ion::gfx::StateTable::kStencilAlways, 1, 0xff);
        ion_state_table_->SetStencilOperations(
            ion::gfx::StateTable::kStencilKeep,
            ion::gfx::StateTable::kStencilKeep,
            ion::gfx::StateTable::kStencilReplace,
            ion::gfx::StateTable::kStencilKeep,
            ion::gfx::StateTable::kStencilKeep,
            ion::gfx::StateTable::kStencilReplace);
    }
    if (use_stencil_) {
        ion_state_table_->Enable(Capability_::kStencilTest, true);
        ion_state_table_->SetColorWriteMasks(true, true, true, true);
        ion_state_table_->SetDepthWriteMask(true);
        ion_state_table_->SetStencilWriteMasks(0, 0);
        ion_state_table_->SetStencilFunctions(
            ion::gfx::StateTable::kStencilNotEqual, 0, 0xff,
            ion::gfx::StateTable::kStencilNotEqual, 0, 0xff);
    }

    return ion_state_table_;
}

}  // namespace SG
