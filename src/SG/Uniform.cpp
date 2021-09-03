#include "SG/Uniform.h"

namespace SG {

void Uniform::AddFields() {
    AddField(count_);
    AddField(float_val_);
    AddField(int_val_);
    AddField(uint_val_);
    AddField(vec2f_val_);
    AddField(vec3f_val_);
    AddField(vec4f_val_);
    AddField(vec2i_val_);
    AddField(vec3i_val_);
    AddField(vec4i_val_);
    AddField(vec2ui_val_);
    AddField(vec3ui_val_);
    AddField(vec4ui_val_);
    AddField(mat2_val_);
    AddField(mat3_val_);
    AddField(mat4_val_);
}

void Uniform::SetUpIon(IonContext &context) {
    if (! ion_uniform_.IsValid()) {
        ion_uniform_ = count_ > 1 ?
            CreateIonArrayUniform_(*context.registry_stack.top()) :
            CreateIonUniform_(*context.registry_stack.top());
    }
}

#if XXXX // Figure this out...
ion::gfx::Uniform Uniform::CreateIonUniform_(
    const ion::gfx::ShaderInputRegistry &reg) const {
    ion::gfx::Uniform u;
    const std::string &name = GetName();

    if      (last_field_set_ == "float_val")
        u = reg.Create<ion::gfx::Uniform>(name, float_val_);
    else if (last_field_set_ == "int_val")
        u = reg.Create<ion::gfx::Uniform>(name, int_val_);
    else if (last_field_set_ == "uint_val")
        u = reg.Create<ion::gfx::Uniform>(name, uint_val_);
    else if (last_field_set_ == "vec2f_val")
        u = reg.Create<ion::gfx::Uniform>(name, vec2f_val_);
    else if (last_field_set_ == "vec3f_val")
        u = reg.Create<ion::gfx::Uniform>(name, vec3f_val_);
    else if (last_field_set_ == "vec4f_val")
        u = reg.Create<ion::gfx::Uniform>(name, vec4f_val_);
    else if (last_field_set_ == "vec2i_val")
        u = reg.Create<ion::gfx::Uniform>(name, vec2i_val_);
    else if (last_field_set_ == "vec3i_val")
        u = reg.Create<ion::gfx::Uniform>(name, vec3i_val_);
    else if (last_field_set_ == "vec4i_val")
        u = reg.Create<ion::gfx::Uniform>(name, vec4i_val_);
    else if (last_field_set_ == "vec2ui_val")
        u = reg.Create<ion::gfx::Uniform>(name, vec2ui_val_);
    else if (last_field_set_ == "vec3ui_val")
        u = reg.Create<ion::gfx::Uniform>(name, vec3ui_val_);
    else if (last_field_set_ == "vec4ui_val")
        u = reg.Create<ion::gfx::Uniform>(name, vec4ui_val_);
    else if (last_field_set_ == "mat2_val")
        u = reg.Create<ion::gfx::Uniform>(name, mat2_val_);
    else if (last_field_set_ == "mat3_val")
        u = reg.Create<ion::gfx::Uniform>(name, mat3_val_);
    else if (last_field_set_ == "mat4_val")
        u = reg.Create<ion::gfx::Uniform>(name, mat4_val_);

    return u;
}

ion::gfx::Uniform Uniform::CreateIonArrayUniform_(
    const ion::gfx::ShaderInputRegistry &reg) const {
    const std::string &name = GetName();
    const int count = count_;

    // This creates a vector of N=count copies of the value and stores them in
    // the array uniform.
    auto create_func = [&reg, &name, count](const auto &val){
        return reg.CreateArrayUniform(
            name, std::vector(count, val).data(), count,
            ion::base::AllocatorPtr()); };

    ion::gfx::Uniform u;
    if      (last_field_set_ == "float_val")
        u = create_func(float_val_);
    else if (last_field_set_ == "int_val")
        u = create_func(int_val_);
    else if (last_field_set_ == "uint_val")
        u = create_func(uint_val_);
    else if (last_field_set_ == "vec2f_val")
        u = create_func(vec2f_val_);
    else if (last_field_set_ == "vec3f_val")
        u = create_func(vec3f_val_);
    else if (last_field_set_ == "vec4f_val")
        u = create_func(vec4f_val_);
    else if (last_field_set_ == "vec2i_val")
        u = create_func(vec2i_val_);
    else if (last_field_set_ == "vec3i_val")
        u = create_func(vec3i_val_);
    else if (last_field_set_ == "vec4i_val")
        u = create_func(vec4i_val_);
    else if (last_field_set_ == "vec2ui_val")
        u = create_func(vec2ui_val_);
    else if (last_field_set_ == "vec3ui_val")
        u = create_func(vec3ui_val_);
    else if (last_field_set_ == "vec4ui_val")
        u = create_func(vec4ui_val_);
    else if (last_field_set_ == "mat2_val")
        u = create_func(mat2_val_);
    else if (last_field_set_ == "mat3_val")
        u = create_func(mat3_val_);
    else if (last_field_set_ == "mat4_val")
        u = create_func(mat4_val_);

    return u;
}
#endif

}  // namespace SG
