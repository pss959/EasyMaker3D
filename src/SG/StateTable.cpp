#include "SG/StateTable.h"

#include "SG/SpecBuilder.h"

namespace SG {

void StateTable::SetUpIon(IonContext &context) {
    if (! ion_state_table_) {
        ion_state_table_.Reset(new ion::gfx::StateTable);

        // The clear_color_ and clear_depth_ fields are set to negative
        // values. If they are still negative, do not set them in the Ion
        // StateTable.
        if (clear_color_[0] >= 0.f)
            ion_state_table_->SetClearColor(clear_color_);
        if (clear_depth_ >= 0.f)
            ion_state_table_->SetClearDepthValue(clear_depth_);

        ion_state_table_->Enable(Capability_::kDepthTest, depth_test_enabled_);
        ion_state_table_->Enable(Capability_::kCullFace,  cull_face_enabled_);
        ion_state_table_->SetCullFaceMode(cull_face_mode_);
    }
}

Parser::ObjectSpec StateTable::GetObjectSpec() {
    SG::SpecBuilder<StateTable> builder;
    builder.AddVector4f("clear_color",    &StateTable::clear_color_);
    builder.AddFloat("clear_depth",       &StateTable::clear_depth_);
    builder.AddBool("depth_test_enabled", &StateTable::depth_test_enabled_);
    builder.AddBool("cull_face_enabled",  &StateTable::cull_face_enabled_);
    builder.AddEnum<CullFaceMode>("cull_face_mode",
                                  &StateTable::cull_face_mode_);
    return Parser::ObjectSpec{
        "StateTable", false, []{ return new StateTable; }, builder.GetSpecs() };
}

}  // namespace SG
