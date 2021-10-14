#include "SG/Uniform.h"

#include "Assert.h"

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

void Uniform::SetFieldName(const std::string &name) {
    ASSERT(last_field_set_.empty());
    last_field_set_ = name;
}

size_t Uniform::SetUpIon(const ion::gfx::ShaderInputRegistry &reg,
                         ion::gfx::UniformBlock &block) {
    if (ion_index_ == ion::base::kInvalidIndex) {
        IonUniform iu =
            count_ > 1 ? CreateIonArrayUniform_(reg) : CreateIonUniform_(reg);
        ASSERTM(iu.IsValid(), GetDesc());

        ion_index_ = block.AddUniform(iu);
        ASSERTM(ion_index_ != ion::base::kInvalidIndex, GetDesc());
    }
    return ion_index_;
}

// Instantiate SetValue() for all types.
#define SET_VALUE_(TYPE, FIELD, FIELD_NAME)                             \
template <> void Uniform::SetValue(const TYPE &value) {                 \
    ASSERT(last_field_set_.empty() || last_field_set_ == FIELD_NAME);   \
    last_field_set_ = FIELD_NAME;                                       \
    FIELD = value;                                                      \
}

SET_VALUE_(float,        float_val_,  "float_val")
SET_VALUE_(int,          int_val_,    "int_val")
SET_VALUE_(unsigned int, uint_val_,   "uint_val")
SET_VALUE_(Vector2f,     vec2f_val_,  "vec2f_val")
SET_VALUE_(Vector3f,     vec3f_val_,  "vec3f_val")
SET_VALUE_(Vector4f,     vec4f_val_,  "vec4f_val")
SET_VALUE_(Vector2i,     vec2i_val_,  "vec2i_val")
SET_VALUE_(Vector3i,     vec3i_val_,  "vec3i_val")
SET_VALUE_(Vector4i,     vec4i_val_,  "vec4i_val")
SET_VALUE_(Vector2ui,    vec2ui_val_, "vec2ui_val")
SET_VALUE_(Vector3ui,    vec3ui_val_, "vec3ui_val")
SET_VALUE_(Vector4ui,    vec4ui_val_, "vec4ui_val")
SET_VALUE_(Matrix2f,     mat2_val_,   "mat2_val")
SET_VALUE_(Matrix3f,     mat3_val_,   "mat3_val")
SET_VALUE_(Matrix4f,     mat4_val_,   "mat4_val")

#undef SET_VALUE_

void Uniform::CopyContentsFrom(const Parser::Object &from, bool is_deep) {
    ASSERT(from.GetTypeName() == "Uniform");
    const Uniform &from_uniform = static_cast<const Uniform &>(from);
    last_field_set_ = from_uniform.last_field_set_;
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
