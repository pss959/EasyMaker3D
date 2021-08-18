#include "SG/StateTable.h"

#include "SG/SpecBuilder.h"

namespace SG {

void StateTable::Finalize() {
    assert(! ion_state_table_);
    ion_state_table_.Reset(new ion::gfx::StateTable);
    ion_state_table_->SetClearColor(clear_color_);
    ion_state_table_->Enable(Capability_::kDepthTest, depth_test_enabled_);
    ion_state_table_->Enable(Capability_::kCullFace,  cull_face_enabled_);
}

NParser::ObjectSpec StateTable::GetObjectSpec() {
    SG::SpecBuilder<StateTable> builder;
    builder.AddVector4f("clear_color",    &StateTable::clear_color_);
    builder.AddBool("depth_test_enabled", &StateTable::depth_test_enabled_);
    builder.AddBool("cull_face_enabled",  &StateTable::cull_face_enabled_);
    return NParser::ObjectSpec{
        "StateTable", []{ return new StateTable; }, builder.GetSpecs() };
}

}  // namespace SG
