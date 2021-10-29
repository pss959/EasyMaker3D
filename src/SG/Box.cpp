#include "SG/Box.h"

#include <ion/gfxutils/shapeutils.h>

#include "Math/Intersection.h"

namespace SG {

void Box::AddFields() {
    AddField(size_);
    PrimitiveShape::AddFields();
}

bool Box::IntersectRay(const Ray &ray, Hit &hit) const {
    // Compensate for shape transformations.
    const Ray local_ray = GetLocalRay(ray);

    float        distance;
    Bounds::Face face;
    bool         is_entry;
    if (RayBoundsIntersectFace(local_ray, ComputeBounds(), distance,
                               face, is_entry) && is_entry) {
        hit.distance = distance;
        hit.point    = local_ray.GetPoint(distance);
        hit.normal   = Bounds::GetFaceNormal(face);
        return true;
    }
    return false;
}

Bounds Box::GetUntransformedBounds() const {
    return Bounds(size_);
}

ion::gfx::ShapePtr Box::CreateSpecificIonShape() {
    ion::gfxutils::BoxSpec spec;
    if (size_.WasSet())
        spec.size = size_;
    UpdateShapeSpec(spec);
    return ion::gfxutils::BuildBoxShape(spec);
}

}  // namespace SG
