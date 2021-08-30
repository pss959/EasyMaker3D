#include "SG/Rectangle.h"

#include <ion/gfxutils/shapeutils.h>

#include "SG/SpecBuilder.h"

namespace SG {

Bounds Rectangle::ComputeBounds() const {
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
        size.Set(size_[0], size_[1], .001f);
        break;
    }
    return Bounds(size);
}

bool Rectangle::IntersectRay(const Ray &ray, Hit &hit) const {
    // Intersect with the rectangle's plane.
    Vector3f normal;
    switch (plane_normal_) {
      case PlaneNormal::kPositiveX: normal.Set( 1,  0,  0); break;
      case PlaneNormal::kNegativeX: normal.Set(-1,  0,  0); break;
      case PlaneNormal::kPositiveY: normal.Set( 0,  1,  0); break;
      case PlaneNormal::kNegativeY: normal.Set( 0, -1,  0); break;
      case PlaneNormal::kPositiveZ: normal.Set( 0,  0,  1); break;
      case PlaneNormal::kNegativeZ: normal.Set( 0,  0, -1); break;
    }
    float distance;
    if (! RayPlaneIntersect(ray, Plane(0.f, normal), distance))
        return false;

    // Assume the intersection point is within the rectangle, since it must
    // have hit the very thin bounds.
    hit.distance = distance;
    hit.point    = ray.GetPoint(distance);
    hit.normal   = normal;
    return true;
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
