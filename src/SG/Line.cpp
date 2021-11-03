#include "SG/Line.h"

#include <ion/gfxutils/shapeutils.h>

namespace SG {

void Line::AddFields() {
    AddField(end0_);
    AddField(end1_);
    PrimitiveShape::AddFields();
}

void Line::SetEndpoints(const Point3f &end0, const Point3f &end1) {
    end0_ = end0;
    end1_ = end1;

    if (GetIonShape()) {
        // Build a new version of the shape.
        ion::gfx::ShapePtr new_shape = CreateSpecificIonShape();

        // Swap attribute arrays.
        GetIonShape()->SetAttributeArray(new_shape->GetAttributeArray());
    }

    GetChanged().Notify(Change::kGeometry);
}

bool Line::IntersectRay(const Ray &ray, Hit &hit) const {
    return false;  // Lines cannot be intersected.
}

Bounds Line::GetUntransformedBounds() const {
    Bounds bounds;
    bounds.ExtendByPoint(end0_);
    bounds.ExtendByPoint(end1_);
    return bounds;
}

ion::gfx::ShapePtr Line::CreateSpecificIonShape() {
    ion::gfxutils::PolyLineSpec spec;
    spec.vertices.push_back(end0_);
    spec.vertices.push_back(end1_);
    // No normals or tex coords.
    spec.vertex_type = ion::gfxutils::ShapeSpec::kPosition;
    // Allow changes to be made.
    spec.usage_mode = ion::gfx::BufferObject::kDynamicDraw;
    UpdateShapeSpec(spec);
    return ion::gfxutils::BuildPolyLineShape(spec);
}

}  // namespace SG
