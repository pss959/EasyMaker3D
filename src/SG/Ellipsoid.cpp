#include "SG/Ellipsoid.h"

#include <ion/gfxutils/shapeutils.h>
#include <ion/math/transformutils.h>

#include "Math/Intersection.h"
#include "Math/Linear.h"
#include "SG/SpecBuilder.h"

namespace SG {

Bounds Ellipsoid::ComputeBounds() const {
    // Ignore long/lat angles here.
    return Bounds(size_);
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

ion::gfx::ShapePtr Ellipsoid::CreateIonShape() {
    ion::gfxutils::EllipsoidSpec spec;
    spec.longitude_start = longitude_start_;
    spec.longitude_end   = longitude_end_;
    spec.latitude_start  = latitude_start_;
    spec.latitude_end    = latitude_end_;
    spec.band_count      = band_count_;
    spec.sector_count    = sector_count_;
    spec.size            = size_;
    return ion::gfxutils::BuildEllipsoidShape(spec);
}

Parser::ObjectSpec Ellipsoid::GetObjectSpec() {
    SG::SpecBuilder<Ellipsoid> builder;
    builder.AddAnglef("longitude_start", &Ellipsoid::longitude_start_);
    builder.AddAnglef("longitude_end",   &Ellipsoid::longitude_end_);
    builder.AddAnglef("latitude_start",  &Ellipsoid::latitude_start_);
    builder.AddAnglef("latitude_end",    &Ellipsoid::latitude_end_);
    builder.AddInt("band_count",         &Ellipsoid::band_count_);
    builder.AddInt("sector_count",       &Ellipsoid::sector_count_);
    builder.AddVector3f("size",          &Ellipsoid::size_);
    return Parser::ObjectSpec{
        "Ellipsoid", false, []{ return new Ellipsoid; }, builder.GetSpecs() };
}

}  // namespace SG
