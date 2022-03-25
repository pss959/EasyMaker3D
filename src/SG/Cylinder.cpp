#include "SG/Cylinder.h"

#include <algorithm>

#include <ion/gfxutils/shapeutils.h>
#include <ion/math/angleutils.h>

#include "Math/Intersection.h"
#include "SG/Hit.h"

namespace SG {

void Cylinder::AddFields() {
    AddField(bottom_radius_);
    AddField(top_radius_);
    AddField(height_);
    AddField(has_top_cap_);
    AddField(has_bottom_cap_);
    AddField(shaft_band_count_);
    AddField(cap_band_count_);
    AddField(sector_count_);
    PrimitiveShape::AddFields();
}

Bounds Cylinder::GetUntransformedBounds() const {
    const float max_diameter = 2.f * std::max(bottom_radius_, top_radius_);
    return Bounds(Vector3f(max_diameter, height_, max_diameter));
}

bool Cylinder::IntersectUntransformedRay(const Ray &ray, Hit &hit) const {
    // Cap intersection.
    auto test_cap = [&](bool has, float y, float rad, float &d, Point3f &p){
        if (has &&
            RayPlaneIntersect(ray, Plane(y, Vector3f::AxisY()), d)) {
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
    // Intersect with the cylindrical part. If the radii are the same, it is a
    // true cylinder, which is fairly straightforward.
    if (top_radius_ == bottom_radius_) {
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
    else {
        // Assume the top radius is smaller. If not, flip the results later.
        const float r_min = std::min(top_radius_, bottom_radius_);
        const float r_max = std::max(top_radius_, bottom_radius_);

        // Compute d, the height of the apex above the top cap, and the
        // half-angle at the apex.
        float d;
        if (r_min == 0.f) {  // True cone already!
            d = 0;
        }
        else {
            // Truncated cone. Call the height h, the top radius t, the bottom
            // radius b, and the y distance from the top cap to the apex d.
            // Using similar triangles:
            //    d/t = (d + h)/b
            // So   d = (h * r) / (1 - r)  where r = t/b
            const float r = r_min / r_max;
            d = (height_ * r) / (1.f - r);
        }
        const Anglef half_angle = ion::math::ArcTangent2(r_max, height_ + d);
        float apex_y = half_height + d;

        // Reverse if upside down.
        Vector3f axis = Vector3f(0, -1, 0);
        if (r_min == bottom_radius_) {
            apex_y  = -apex_y;
            axis[1] = 1;
        }

        const Point3f apex(0, apex_y, 0);
        if (RayConeIntersect(ray, apex, axis, half_angle, distance) &&
            (! got_hit || distance < hit.distance)) {
            pt = ray.GetPoint(distance);
            if (pt[1] >= -half_height && pt[1] <= half_height) {
                using ion::math::Cross;
                using ion::math::Normalized;
                const Vector3f side = Normalized(pt - apex);
                hit.distance = distance;
                hit.point    = pt;
                hit.normal   = Normalized(Cross(Cross(axis, side), side));
                got_hit = true;
            }
        }
    }

    return got_hit;
}

ion::gfx::ShapePtr Cylinder::CreateSpecificIonShape() {
    ion::gfxutils::CylinderSpec spec;
    if (bottom_radius_.WasSet())
        spec.bottom_radius    = bottom_radius_;
    if (top_radius_.WasSet())
        spec.top_radius       = top_radius_;
    if (height_.WasSet())
        spec.height           = height_;
    if (has_top_cap_.WasSet())
        spec.has_top_cap      = has_top_cap_;
    if (has_bottom_cap_.WasSet())
        spec.has_bottom_cap   = has_bottom_cap_;
    if (shaft_band_count_.WasSet())
        spec.shaft_band_count = shaft_band_count_;
    if (cap_band_count_.WasSet())
        spec.cap_band_count   = cap_band_count_;
    if (sector_count_.WasSet())
        spec.sector_count     = sector_count_;
    // Need to access the attribute data.
    spec.usage_mode = ion::gfx::BufferObject::kDynamicDraw;
    UpdateShapeSpec(spec);
    ion::gfx::ShapePtr shape = ion::gfxutils::BuildCylinderShape(spec);
    return shape;
}

}  // namespace SG
