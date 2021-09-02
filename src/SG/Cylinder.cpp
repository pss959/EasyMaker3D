#include "SG/Cylinder.h"

#include <algorithm>

#include <ion/gfxutils/shapeutils.h>

#include "Math/Intersection.h"
#include "SG/SpecBuilder.h"

namespace SG {

Bounds Cylinder::ComputeBounds() const {
    const float max_diameter = 2.f * std::max(bottom_radius_, top_radius_);
    return Bounds(Vector3f(max_diameter, height_, max_diameter));
}

bool Cylinder::IntersectRay(const Ray &ray, Hit &hit) const {
    // Cap intersection.
    auto test_cap = [ray](bool has, float y, float rad, float &d, Point3f &p){
        if (has && RayPlaneIntersect(ray, Plane(y, Vector3f::AxisY()), d)) {
            p = ray.GetPoint(d);
            if (p[0] * p[0] + p[2] * p[2] < rad * rad)
                return true;
        }
        return false;
    };

    // Intersect with the caps, if any.
    const float half_height = .5f * height_;
    float distance;
    Point3f pt;
    bool got_hit = false;
    if (test_cap(has_top_cap_, half_height, top_radius_, distance, pt)) {
        hit.distance = distance;
        hit.point    = pt;
        hit.normal.Set(0, 1, 0);
        got_hit = true;
    }
    if (test_cap(has_bottom_cap_, -half_height, bottom_radius_,
                 distance, pt)) {
        if (! got_hit || distance < hit.distance) {
            hit.distance = distance;
            hit.point    = pt;
            hit.normal.Set(0, -1, 0);
            got_hit = true;
        }
    }
    // Intersect with the cylindrical part.
    if (top_radius_ == bottom_radius_) { // True cylinder.
        if (RayCylinderIntersect(ray, top_radius_, distance) &&
            (! got_hit || distance < hit.distance)) {
            pt = ray.GetPoint(distance);
            if (pt[1] >= -half_height && pt[1] <= half_height) {
                hit.distance = distance;
                hit.point    = pt;
                hit.normal = ion::math::Normalized(Vector3f(pt[0], 0, pt[2]));
                got_hit = true;
            }
        }
    }
    else { // Cone.
        //XXXX
    }

    return got_hit;
}

ion::gfx::ShapePtr Cylinder::CreateIonShape() {
    ion::gfxutils::CylinderSpec spec;
    spec.bottom_radius    = bottom_radius_;
    spec.top_radius       = top_radius_;
    spec.height           = height_;
    spec.has_top_cap      = has_top_cap_;
    spec.has_bottom_cap   = has_bottom_cap_;
    spec.shaft_band_count = shaft_band_count_;
    spec.cap_band_count   = cap_band_count_;
    spec.sector_count     = sector_count_;
    // Need to access the attribute data.
    spec.usage_mode = ion::gfx::BufferObject::kDynamicDraw;
    ion::gfx::ShapePtr shape = ion::gfxutils::BuildCylinderShape(spec);
    FillTriMesh(*shape);
    return shape;
}

Parser::ObjectSpec Cylinder::GetObjectSpec() {
    SG::SpecBuilder<Cylinder> builder;
    builder.AddFloat("bottom_radius",  &Cylinder::bottom_radius_);
    builder.AddFloat("top_radius",     &Cylinder::top_radius_);
    builder.AddFloat("height",         &Cylinder::height_);
    builder.AddBool("has_top_cap",     &Cylinder::has_top_cap_);
    builder.AddBool("has_bottom_cap",  &Cylinder::has_bottom_cap_);
    builder.AddInt("shaft_band_count", &Cylinder::shaft_band_count_);
    builder.AddInt("cap_band_count",   &Cylinder::cap_band_count_);
    builder.AddInt("sector_count",     &Cylinder::sector_count_);
    return Parser::ObjectSpec{
        "Cylinder", false, []{ return new Cylinder; }, builder.GetSpecs() };
}

}  // namespace SG
