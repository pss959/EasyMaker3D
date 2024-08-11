//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/RegularPolygon.h"

#include <ion/gfxutils/shapeutils.h>

#include "Util/Tuning.h"

namespace SG {

void RegularPolygon::AddFields() {
    AddField(sides_.Init("sides", 3));
    AddField(plane_normal_.Init("plane_normal", PlaneNormal::kPositiveZ));

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
    size[flat_dim] = TK::kFlatThickness;
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
