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

void PolyLine::SetArcPoints(const CircleArc &arc, float radius,
                            float degrees_per_segment) {
    std::vector<Point3f> points;
    const int seg_count = static_cast<int>(std::abs(arc.arc_angle.Degrees()) /
                                           degrees_per_segment);
    if (seg_count) {
        const Anglef seg_angle = arc.arc_angle / seg_count;
        const int pt_count = 1 + seg_count;
        points.resize(pt_count);
        for (int i = 0; i < pt_count; ++i) {
            const Anglef angle = arc.start_angle + i * seg_angle;
            points[i].Set(radius * ion::math::Cosine(angle),
                          radius * ion::math::Sine(angle), 0);
        }
    }
    SetPoints(points);
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

    // Make sure there are vertices.
    if (spec.vertices.empty()) {
        spec.vertices.push_back(Point3f(0, 0, 0));
        spec.vertices.push_back(Point3f(1, 0, 0));
    }

    // No normals or tex coords.
    spec.vertex_type = ion::gfxutils::ShapeSpec::kPosition;
    // Allow changes to be made.
    spec.usage_mode = ion::gfx::BufferObject::kDynamicDraw;
    UpdateShapeSpec(spec);
    return ion::gfxutils::BuildPolyLineShape(spec);
}

}  // namespace SG
