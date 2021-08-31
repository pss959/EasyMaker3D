#include "SG/Line.h"

#include <ion/gfxutils/shapeutils.h>

#include "SG/SpecBuilder.h"

namespace SG {

void Line::SetEndpoints(const Point3f &end0, const Point3f &end1) {
    end0_ = end0;
    end1_ = end1;

    // Build a new version of the shape.
    ion::gfx::ShapePtr new_shape = CreateIonShape();

    // Swap attribute arrays.
    GetIonShape()->SetAttributeArray(new_shape->GetAttributeArray());

    GetChanged().Notify(Change::kGeometry);
}

Bounds Line::ComputeBounds() const {
    Bounds bounds;
    bounds.ExtendByPoint(end0_);
    bounds.ExtendByPoint(end1_);
    return bounds;
}

bool Line::IntersectRay(const Ray &ray, Hit &hit) const {
    return false;  // Lines cannot be intersected.
}

ion::gfx::ShapePtr Line::CreateIonShape() {
    ion::gfxutils::PolyLineSpec spec;
    spec.vertices.push_back(end0_);
    spec.vertices.push_back(end1_);
    // No normals or tex coords.
    spec.vertex_type = ion::gfxutils::ShapeSpec::kPosition;
    // Allow changes to be made.
    spec.usage_mode = ion::gfx::BufferObject::kDynamicDraw;
    return ion::gfxutils::BuildPolyLineShape(spec);
}

Parser::ObjectSpec Line::GetObjectSpec() {
    SG::SpecBuilder<Line> builder;
    builder.AddPoint3f("end0", &Line::end0_);
    builder.AddPoint3f("end1", &Line::end1_);
    return Parser::ObjectSpec{
        "Line", false, []{ return new Line; }, builder.GetSpecs() };
}

}  // namespace SG
