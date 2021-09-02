#include "SG/Material.h"

#include "SG/SpecBuilder.h"

namespace SG {

Parser::ObjectSpec Material::GetObjectSpec() {
    SG::SpecBuilder<Material> builder;
    builder.AddVector4f("base_color", &Material::base_color_);
    builder.AddFloat("smoothness",    &Material::smoothness_);
    builder.AddFloat("metalness",     &Material::metalness_);
    return Parser::ObjectSpec{
        "Material", false, []{ return new Material; }, builder.GetSpecs() };
}

}  // namespace SG
