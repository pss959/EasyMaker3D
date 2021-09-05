#include "SG/Box.h"

#include <ion/gfxutils/shapeutils.h>

#include "Math/Intersection.h"

namespace SG {

void Box::AddFields() {
    AddField(size_);
}

bool Box::IntersectRay(const Ray &ray, Hit &hit) const {
    float        distance;
    Bounds::Face face;
    bool         is_entry;
    if (RayBoundsIntersectFace(ray, ComputeBounds(), distance,
                               face, is_entry) && is_entry) {
        hit.distance = distance;
        hit.point    = ray.GetPoint(distance);
        hit.normal   = Bounds::GetFaceNormal(face);
    }
    return false;
}

Bounds Box::ComputeBounds() const {
    return Bounds(size_);
}

ion::gfx::ShapePtr Box::CreateIonShape() {
    ion::gfxutils::BoxSpec spec;
    if (size_.WasSet())
        spec.size = size_;
    return ion::gfxutils::BuildBoxShape(spec);
}

}  // namespace SG
