#include "SG/PointLight.h"

#include "SG/SpecBuilder.h"

namespace SG {

Parser::ObjectSpec PointLight::GetObjectSpec() {
    SG::SpecBuilder<PointLight> builder;
    builder.AddPoint3f("position", &PointLight::position_);
    builder.AddVector4f("color",   &PointLight::color_);
    return Parser::ObjectSpec{
        "PointLight", false, []{ return new PointLight; }, builder.GetSpecs() };
}

}  // namespace SG
