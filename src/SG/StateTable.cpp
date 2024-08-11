//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/StateTable.h"

#include "SG/IonContext.h"
#include "Util/KLog.h"

namespace SG {

void StateTable::AddFields() {

    AddField(clear_color_.Init("clear_color", Vector4f::Zero()));
    AddField(clear_depth_.Init("clear_depth", 0));
    AddField(clear_stencil_.Init("clear_stencil", 0));
    AddField(line_width_.Init("line_width", 1));
    AddField(blend_enabled_.Init("blend_enabled", false));
    AddField(depth_test_enabled_.Init("depth_test_enabled", false));
    AddField(cull_face_enabled_.Init("cull_face_enabled", false));
    AddField(cull_face_mode_.Init("cull_face_mode", IGS_::kCullBack));
    AddField(rgb_blend_equation_.Init("rgb_blend_equation", IGS_::kAdd));
    AddField(alpha_blend_equation_.Init("alpha_blend_equation", IGS_::kAdd));
    AddField(rgb_blend_source_factor_.Init("rgb_blend_source_factor",
                                           IGS_::kOne));
    AddField(rgb_blend_dest_factor_.Init("rgb_blend_dest_factor", IGS_::kZero));
    AddField(alpha_blend_source_factor_.Init("alpha_blend_source_factor",
                                             IGS_::kOne));
    AddField(alpha_blend_dest_factor_.Init("alpha_blend_dest_factor",
                                           IGS_::kZero));
    AddField(create_stencil_.Init("create_stencil", false));
    AddField(use_stencil_.Init("use_stencil", false));
    AddField(reset_stencil_.Init("reset_stencil", false));

    Object::AddFields();
}

void StateTable::SetLineWidth(float width) {
    line_width_ = width;
    if (ion_state_table_)
        ion_state_table_->SetLineWidth(line_width_);
}

ion::gfx::StateTablePtr StateTable::SetUpIon(const IonContextPtr &ion_context) {
    ASSERT(! ion_state_table_);
    ion_state_table_.Reset(new ion::gfx::StateTable);
    KLOG('Z', ion_context->GetIndent() << "SetUpIon for " << GetDesc());

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
    if (blend_enabled_.WasSet())
        ion_state_table_->Enable(IGS_::Capability::kBlend, blend_enabled_);
    if (depth_test_enabled_.WasSet())
        ion_state_table_->Enable(IGS_::Capability::kDepthTest,
                                 depth_test_enabled_);
    if (cull_face_enabled_.WasSet())
        ion_state_table_->Enable(IGS_::Capability::kCullFace,
                                 cull_face_enabled_);
    if (cull_face_mode_.WasSet())
        ion_state_table_->SetCullFaceMode(cull_face_mode_);

    if (rgb_blend_equation_.WasSet() ||
        alpha_blend_equation_.WasSet()) {
        ion_state_table_->SetBlendEquations(rgb_blend_equation_,
                                            alpha_blend_equation_);
    }
    if (rgb_blend_source_factor_.WasSet()   ||
        rgb_blend_dest_factor_.WasSet()     ||
        alpha_blend_source_factor_.WasSet() ||
        alpha_blend_dest_factor_.WasSet()) {
        ion_state_table_->SetBlendFunctions(
            rgb_blend_source_factor_,   rgb_blend_dest_factor_,
            alpha_blend_source_factor_, alpha_blend_dest_factor_);
    }

    if (create_stencil_) {
        ion_state_table_->Enable(IGS_::Capability::kStencilTest, true);
        ion_state_table_->SetColorWriteMasks(false, false, false, false);
        ion_state_table_->SetDepthWriteMask(false);
        ion_state_table_->SetStencilWriteMasks(0xffffffff, 0xffffffff);
        ion_state_table_->SetStencilFunctions(IGS_::kStencilAlways, 1, 0xff,
                                              IGS_::kStencilAlways, 1, 0xff);
        ion_state_table_->SetStencilOperations(
            IGS_::kStencilKeep, IGS_::kStencilKeep, IGS_::kStencilReplace,
            IGS_::kStencilKeep, IGS_::kStencilKeep, IGS_::kStencilReplace);
    }
    if (use_stencil_) {
        ion_state_table_->Enable(IGS_::Capability::kStencilTest, true);
        ion_state_table_->SetColorWriteMasks(true, true, true, true);
        ion_state_table_->SetDepthWriteMask(true);
        ion_state_table_->SetStencilWriteMasks(0, 0);
        ion_state_table_->SetStencilFunctions(IGS_::kStencilNotEqual, 0, 0xff,
                                              IGS_::kStencilNotEqual, 0, 0xff);
    }
    if (reset_stencil_) {
        ion_state_table_->Enable(IGS_::Capability::kStencilTest, false);
    }

    return ion_state_table_;
}

}  // namespace SG
