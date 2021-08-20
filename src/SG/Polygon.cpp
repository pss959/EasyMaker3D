#include "SG/Polygon.h"

#include <ion/gfxutils/shapeutils.h>

#include "SG/SpecBuilder.h"

namespace SG {

ion::gfx::ShapePtr Polygon::CreateIonShape() {
    ion::gfxutils::RegularPolygonSpec spec;
    spec.sides        = sides_;
    spec.plane_normal = plane_normal_;
    return ion::gfxutils::BuildRegularPolygonShape(spec);
}

Parser::ObjectSpec Polygon::GetObjectSpec() {
    SG::SpecBuilder<Polygon> builder;
    builder.AddInt("sides", &Polygon::sides_);
    builder.AddEnum<PlaneNormal>("plane_normal", &Polygon::plane_normal_);
    return Parser::ObjectSpec{
        "Polygon", false, []{ return new Polygon; }, builder.GetSpecs() };
}

}  // namespace SG
