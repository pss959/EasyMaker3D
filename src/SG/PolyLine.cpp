#include "SG/PolyLine.h"

#include <ion/gfxutils/shapeutils.h>

namespace SG {

void PolyLine::AddFields() {
    AddField(points_);
    PrimitiveShape::AddFields();
}

void PolyLine::SetPoints(const std::vector<Point3f> &points) {
    points_ = points;

    if (GetIonShape()) {
        // Build a new version of the shape.
        ion::gfx::ShapePtr new_shape = CreateSpecificIonShape();

        // Swap attribute arrays.
        GetIonShape()->SetAttributeArray(new_shape->GetAttributeArray());
    }

    ProcessChange(Change::kGeometry, *this);
}

Bounds PolyLine::GetUntransformedBounds() const {
    Bounds bounds;
    for (const auto &point: GetPoints())
        bounds.ExtendByPoint(point);
    return bounds;
}

bool PolyLine::IntersectUntransformedRay(const Ray &ray, Hit &hit) const {
    return false;  // PolyLines cannot be intersected.
}

ion::gfx::ShapePtr PolyLine::CreateSpecificIonShape() {
    ion::gfxutils::PolyLineSpec spec;
    spec.vertices = points_;
    // No normals or tex coords.
    spec.vertex_type = ion::gfxutils::ShapeSpec::kPosition;
    // Allow changes to be made.
    spec.usage_mode = ion::gfx::BufferObject::kDynamicDraw;
    UpdateShapeSpec(spec);
    return ion::gfxutils::BuildPolyLineShape(spec);
}

}  // namespace SG
