#include "SG/Uniform.h"

#include "SG/SpecBuilder.h"

namespace SG {

void Uniform::SetUpIon(IonContext &context) {
    if (! ion_uniform_.IsValid()) {
        ion_uniform_ = CreateIonUniform_(*context.registry_stack.top());
    }
}

Parser::ObjectSpec Uniform::GetObjectSpec() {
    SG::SpecBuilder<Uniform> builder;
    builder.AddFloat("float_val",      &Uniform::float_val_);
    builder.AddInt("int_val",          &Uniform::int_val_);
    builder.AddUInt("uint_val",        &Uniform::uint_val_);
    builder.AddVector2f("vec2f_val",   &Uniform::vec2f_val_);
    builder.AddVector3f("vec3f_val",   &Uniform::vec3f_val_);
    builder.AddVector4f("vec4f_val",   &Uniform::vec4f_val_);
    builder.AddVector2i("vec2i_val",   &Uniform::vec2i_val_);
    builder.AddVector3i("vec3i_val",   &Uniform::vec3i_val_);
    builder.AddVector4i("vec4i_val",   &Uniform::vec4i_val_);
    builder.AddVector2ui("vec2ui_val", &Uniform::vec2ui_val_);
    builder.AddVector3ui("vec3ui_val", &Uniform::vec3ui_val_);
    builder.AddVector4ui("vec4ui_val", &Uniform::vec4ui_val_);
    builder.AddMatrix2f("mat2_val",    &Uniform::mat2_val_);
    builder.AddMatrix3f("mat3_val",    &Uniform::mat3_val_);
    builder.AddMatrix4f("mat4_val",    &Uniform::mat4_val_);

    // Wrap the FieldSpec store functions with a new function that calls the
    // old one and then saves the name of the last field stored.
    std::vector<Parser::FieldSpec> specs = builder.GetSpecs();
    for (Parser::FieldSpec &spec: specs) {
        const Parser::FieldSpec::StoreFunc old_func = spec.store_func;
        const Parser::FieldSpec::StoreFunc new_func =
            [spec, old_func](Parser::Object &obj,
                             const std::vector<Parser::Value> &vals){
                old_func(obj, vals);
                static_cast<Uniform &>(obj).last_field_set_ = spec.name; };
        spec.store_func = new_func;
    }

    return Parser::ObjectSpec{
        "Uniform", true, []{ return new Uniform; }, specs };
}

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

}  // namespace SG
