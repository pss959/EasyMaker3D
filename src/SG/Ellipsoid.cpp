#include "SG/Ellipsoid.h"

#include <ion/gfxutils/shapeutils.h>
#include <ion/math/transformutils.h>

#include "SG/SpecBuilder.h"

namespace SG {

Bounds Ellipsoid::ComputeBounds() const {
    // Ignore long/lat angles here.
    std::cerr << "XXXX Ellipsoid returns bounds " << Bounds(size_) << "\n";
    return Bounds(size_);
}

bool Ellipsoid::IntersectRay(const Ray &ray, Hit &hit) const {
    // Transform the ray by the inverse of twice the size so that we can use
    // the unit sphere at the origin. It's twice the size because the size
    // represents diameters, not radii.
    Matrix4f inv_scale = ion::math::ScaleMatrixH(2.f / size_);

    const Ray unit_sphere_ray = TransformRay(ray, inv_scale);

    std::cerr << "XXXX Intersecting = " << unit_sphere_ray.origin << " / "
              << unit_sphere_ray.direction
              << " with ellipsoid of size " << size_ << "\n";

    // Let:
    //   r = sphere radius (= 1 for unit sphere).
    //   P = starting point of ray
    //   D = ray direction
    //
    // A point on the ray:
    //   P + t * D   [t > 0]
    //
    // For any point S on the sphere:
    //   || S || == r
    //
    // Therefore:
    //   || (P + t * D) || == r
    //      square both sides
    //   (P + t * D) . (P + t * D) == r*r
    //      expand and refactor to get a quadratic equation:
    //
    //   a * t*t + b * t + c = 0
    //      where
    //         a = D . D
    //         b = 2 * (D . P)
    //         c = P . P - r*r

    // At^2 + Bt + C = 0
    const Vector3f p = unit_sphere_ray.origin - Point3f::Zero();
    const float    a = ion::math::LengthSquared(unit_sphere_ray.direction);
    const float    b = 2.f * ion::math::Dot(unit_sphere_ray.direction, p);
    const float    c = ion::math::LengthSquared(p) - 1.f;

    // If the discriminant is zero or negative, there is no good intersection.
    float discriminant = b * b - 4. * a * c;
    if (discriminant <= 0.f)
	return false;

    // Compute t as:
    //		(-b - sqrt(b^2 - 4c)) / 2a
    //    and   (-b + sqrt(b^2 - 4c)) / 2a
    //
    // Since the sqrt is positive, the first form is never larger than the
    // second, so see if the first one is valid.
    const float sqroot = std::sqrt(discriminant);
    float t = (-b - sqroot) / (2.f * a);
    if (t <= 0.)
        t = (-b + sqroot) / (2.f * a);
    if (t < 0.)
        return false;

    Point3f pt = ray.GetPoint(t);
    std::cerr << "XXXX  Ell t=" << t << " pt=" << pt << "\n";
    hit.distance = t;
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
