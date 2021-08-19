#include "SG/Uniform.h"

#include "SG/SpecBuilder.h"

namespace SG {

void Uniform::Finalize() {
    // XXXX
}

NParser::ObjectSpec Uniform::GetObjectSpec() {
    SG::SpecBuilder<Uniform> builder;
    builder.AddFloat("float_val",      &Uniform::float_val_);
    builder.AddInt("int_val",          &Uniform::int_val);
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
    std::vector<NParser::FieldSpec> specs = builder.GetSpecs();
    for (NParser::FieldSpec &spec: specs) {
        const NParser::FieldSpec::StoreFunc old_func = spec.store_func;
        const NParser::FieldSpec::StoreFunc new_func =
            [spec, old_func](NParser::Object &obj,
                             const std::vector<NParser::Value> &vals){
                old_func(obj, vals);
                static_cast<Uniform &>(obj).last_field_set_ = spec.name; };
        spec.store_func = new_func;
    }

    return NParser::ObjectSpec{
        "Uniform", true, []{ return new Uniform; }, specs };
}

}  // namespace SG
