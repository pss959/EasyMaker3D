#include "SG/RegularPolygon.h"

#include <ion/gfxutils/shapeutils.h>

namespace SG {

void RegularPolygon::AddFields() {
    AddField(sides_);
    AddField(plane_normal_);
    TriMeshShape::AddFields();
}

Bounds RegularPolygon::ComputeBounds() const {
    int flat_dim =
        plane_normal_ == PlaneNormal::kPositiveX ||
        plane_normal_ == PlaneNormal::kNegativeX ? 0 :
        (plane_normal_ == PlaneNormal::kPositiveY ||
         plane_normal_ == PlaneNormal::kNegativeY ? 1 : 2);

    // Radius is always 1. Use a little thickness in the flat dimension.
    Vector3f size(2.f, 2.f, 2.f);
    size[flat_dim] = .001f;
    return Bounds(size);
}

ion::gfx::ShapePtr RegularPolygon::CreateSpecificIonShape() {
    ion::gfxutils::RegularPolygonSpec spec;
    if (sides_.WasSet())
       spec.sides = sides_;
    if (plane_normal_.WasSet())
        spec.plane_normal = plane_normal_;
    ion::gfx::ShapePtr shape = ion::gfxutils::BuildRegularPolygonShape(spec);
    FillTriMesh(*shape);
    return shape;
}

}  // namespace SG
