#include "SG/Ellipsoid.h"

#include <ion/gfxutils/shapeutils.h>
#include <ion/math/transformutils.h>

#include "Math/Intersection.h"
#include "Math/Linear.h"

namespace SG {

void Ellipsoid::AddFields() {
    AddField(longitude_start_);
    AddField(longitude_end_);
    AddField(latitude_start_);
    AddField(latitude_end_);
    AddField(band_count_);
    AddField(sector_count_);
    AddField(size_);
}

bool Ellipsoid::IntersectRay(const Ray &ray, Hit &hit) const {
    // Transform the ray by the inverse of twice the size so that we can use
    // the unit sphere at the origin. It's twice the size because the size
    // represents diameters, not radii.
    Matrix4f inv_scale = ion::math::ScaleMatrixH(2.f / size_);
    const Ray unit_sphere_ray = TransformRay(ray, inv_scale);
    float distance;
    if (! RaySphereIntersect(unit_sphere_ray, distance))
        return false;
    Point3f pt = ray.GetPoint(distance);
    hit.distance = distance;
    hit.point    = pt;
    hit.normal   = ion::math::Normalized(pt - Point3f::Zero());
    return true;
}

Bounds Ellipsoid::ComputeBounds() const {
    // Ignore long/lat angles here.
    return Bounds(size_);
}

ion::gfx::ShapePtr Ellipsoid::CreateIonShape() {
    ion::gfxutils::EllipsoidSpec spec;
    if (longitude_start_.WasSet())
        spec.longitude_start = longitude_start_;
    if (longitude_end_.WasSet())
        spec.longitude_end   = longitude_end_;
    if (latitude_start_.WasSet())
        spec.latitude_start  = latitude_start_;
    if (latitude_end_.WasSet())
        spec.latitude_end    = latitude_end_;
    if (band_count_.WasSet())
        spec.band_count      = band_count_;
    if (sector_count_.WasSet())
        spec.sector_count    = sector_count_;
    if (size_.WasSet())
        spec.size            = size_;
    return ion::gfxutils::BuildEllipsoidShape(spec);
}

}  // namespace SG
