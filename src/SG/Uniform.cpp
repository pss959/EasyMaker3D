#include "SG/Uniform.h"

using ion::gfx::ShaderInputRegistry;
using IonUniform = ion::gfx::Uniform;

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

IonUniform Uniform::CreateIonUniform(const ShaderInputRegistry &reg) const {
    return count_ > 1 ? CreateIonArrayUniform_(reg) : CreateIonUniform_(reg);
}

IonUniform Uniform::CreateIonUniform_(const ShaderInputRegistry &reg) const {
    IonUniform u;
    const std::string &name = GetName();

#define TEST_(NAME) if (last_field_set_ == #NAME)                       \
        u = reg.Create<IonUniform>(name, NAME ## _.GetValue())

    TEST_(float_val);
    else TEST_(int_val);
    else TEST_(uint_val);
    else TEST_(vec2f_val);
    else TEST_(vec3f_val);
    else TEST_(vec4f_val);
    else TEST_(vec2i_val);
    else TEST_(vec3i_val);
    else TEST_(vec4i_val);
    else TEST_(vec2ui_val);
    else TEST_(vec3ui_val);
    else TEST_(vec4ui_val);
    else TEST_(mat2_val);
    else TEST_(mat3_val);
    else TEST_(mat4_val);

#undef TEST_

    return u;
}

IonUniform Uniform::CreateIonArrayUniform_(
    const ShaderInputRegistry &reg) const {
    const std::string &name = GetName();
    const int count = count_;

    IonUniform u;

    // This creates a vector of N=count copies of the value and stores them in
    // the array uniform.
    auto create_func = [&reg, &name, count](const auto &val){
        return reg.CreateArrayUniform(
            name, std::vector(count, val.GetValue()).data(), count,
            ion::base::AllocatorPtr()); };

#define TEST_(NAME) if (last_field_set_ == #NAME) u = create_func(NAME ## _)

    TEST_(float_val);
    else TEST_(int_val);
    else TEST_(uint_val);
    else TEST_(vec2f_val);
    else TEST_(vec3f_val);
    else TEST_(vec4f_val);
    else TEST_(vec2i_val);
    else TEST_(vec3i_val);
    else TEST_(vec4i_val);
    else TEST_(vec2ui_val);
    else TEST_(vec3ui_val);
    else TEST_(vec4ui_val);
    else TEST_(mat2_val);
    else TEST_(mat3_val);
    else TEST_(mat4_val);

#undef TEST_

    return u;
}

}  // namespace SG
