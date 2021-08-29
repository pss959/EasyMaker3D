#include "SG/Polygon.h"

#include <ion/gfxutils/shapeutils.h>

#include "SG/SpecBuilder.h"

namespace SG {

Bounds Polygon::ComputeBounds() const {
    int flat_dim =
        plane_normal_ == PlaneNormal::kPositiveX ||
        plane_normal_ == PlaneNormal::kNegativeX ? 0 :
        (plane_normal_ == PlaneNormal::kPositiveY ||
         plane_normal_ == PlaneNormal::kNegativeY ? 1 : 2);

    // Radius is always 1. Use a little thickness in the flat dimension.
    Vector3f size(2.f, 2.f, 2.f);
    size[flat_dim] = .01f;
    return Bounds(size);
}

bool Polygon::IntersectRay(const Ray &ray, Hit &hit) const {
    return false; // XXXX
}

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
