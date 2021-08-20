#include "SG/Rectangle.h"

#include <ion/gfxutils/shapeutils.h>

#include "SG/SpecBuilder.h"

namespace SG {

ion::gfx::ShapePtr Rectangle::CreateIonShape() {
    ion::gfxutils::RectangleSpec spec;
    spec.size         = size_;
    spec.plane_normal = plane_normal_;
    return ion::gfxutils::BuildRectangleShape(spec);
}

Parser::ObjectSpec Rectangle::GetObjectSpec() {
    SG::SpecBuilder<Rectangle> builder;
    builder.AddVector2f("size", &Rectangle::size_);
    builder.AddEnum<PlaneNormal>("plane_normal", &Rectangle::plane_normal_);
    return Parser::ObjectSpec{
        "Rectangle", false, []{ return new Rectangle; }, builder.GetSpecs() };
}

}  // namespace SG
