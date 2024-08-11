//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/Box.h"

#include <ion/gfxutils/shapeutils.h>

#include "Math/Intersection.h"
#include "SG/Hit.h"

namespace SG {

void Box::AddFields() {
    AddField(size_.Init("size", Vector3f(1, 1, 1)));

    PrimitiveShape::AddFields();
}

Bounds Box::GetUntransformedBounds() const {
    return Bounds(size_);
}

bool Box::IntersectUntransformedRay(const Ray &ray, Hit &hit) const {
    float        distance;
    Bounds::Face face;
    bool         is_entry;
    if (RayBoundsIntersectFace(ray, GetUntransformedBounds(), distance,
                               face, is_entry) && is_entry) {
        hit.distance = distance;
        hit.point    = ray.GetPoint(distance);
        hit.normal   = Bounds::GetFaceNormal(face);
        return true;
    }
    return false;
}

ion::gfx::ShapePtr Box::CreateSpecificIonShape() {
    ion::gfxutils::BoxSpec spec;
    if (size_.WasSet())
        spec.size = size_;
    UpdateShapeSpec(spec);
    return ion::gfxutils::BuildBoxShape(spec);
}

}  // namespace SG
