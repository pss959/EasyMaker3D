#include "SG/Box.h"

#include <ion/gfxutils/shapeutils.h>

#include "Math/Intersection.h"
#include "SG/SpecBuilder.h"

namespace SG {

Bounds Box::ComputeBounds() const {
    return Bounds(size_);
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

ion::gfx::ShapePtr Box::CreateIonShape() {
    ion::gfxutils::BoxSpec spec;
    spec.size = size_;
    return ion::gfxutils::BuildBoxShape(spec);
}

Parser::ObjectSpec Box::GetObjectSpec() {
    SG::SpecBuilder<Box> builder;
    builder.AddVector3f("size", &Box::size_);
    return Parser::ObjectSpec{
        "Box", false, []{ return new Box; }, builder.GetSpecs() };
}

}  // namespace SG
