#include "SG/Rectangle.h"

#include <ion/gfxutils/shapeutils.h>

#include "SG/SpecBuilder.h"

namespace SG {

Bounds Rectangle::ComputeBounds() {
    Vector3f size;
    switch (plane_normal_) {
      case PlaneNormal::kPositiveX:
      case PlaneNormal::kNegativeX:
        size.Set(.01f, size_[0], size_[1]);
        break;
      case PlaneNormal::kPositiveY:
      case PlaneNormal::kNegativeY:
        size.Set(size_[0], .01f, size_[1]);
        break;
      case PlaneNormal::kPositiveZ:
      case PlaneNormal::kNegativeZ:
        size.Set(size_[0], size_[1], .01f);
        break;
    }
    return Bounds(size);
}

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
