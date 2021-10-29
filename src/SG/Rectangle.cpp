#include "SG/Rectangle.h"

#include <ion/gfxutils/shapeutils.h>

#include "Math/Intersection.h"

namespace SG {

void Rectangle::AddFields() {
    AddField(size_);
    AddField(plane_normal_);
    PrimitiveShape::AddFields();
}

bool Rectangle::IntersectRay(const Ray &ray, Hit &hit) const {
    // Compensate for shape transformations.
    const Ray local_ray = GetLocalRay(ray);

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
    if (! RayPlaneIntersect(local_ray, Plane(0.f, normal), distance))
        return false;

    // Assume the intersection point is within the rectangle, since it must
    // have hit the very thin bounds.
    hit.distance = distance;
    hit.point    = ray.GetPoint(distance);
    hit.normal   = normal;
    return true;
}

Bounds Rectangle::GetUntransformedBounds() const {
    const Vector2f &size2 = GetSize();
    Vector3f size3;
    switch (plane_normal_) {
      case PlaneNormal::kPositiveX:
      case PlaneNormal::kNegativeX:
        size3.Set(.01f, size2[0], size2[1]);
        break;
      case PlaneNormal::kPositiveY:
      case PlaneNormal::kNegativeY:
        size3.Set(size2[0], .01f, size2[1]);
        break;
      case PlaneNormal::kPositiveZ:
      case PlaneNormal::kNegativeZ:
        size3.Set(size2[0], size2[1], .001f);
        break;
    }
    return Bounds(size3);
}

ion::gfx::ShapePtr Rectangle::CreateSpecificIonShape() {
    ion::gfxutils::RectangleSpec spec;
    if (size_.WasSet())
        spec.size = size_;
    if (plane_normal_.WasSet())
        spec.plane_normal = plane_normal_;
    UpdateShapeSpec(spec);
    return ion::gfxutils::BuildRectangleShape(spec);
}

}  // namespace SG
