#include "SG/Ellipsoid.h"

#include <ion/gfxutils/shapeutils.h>

#include "SG/SpecBuilder.h"

namespace SG {

Bounds Ellipsoid::ComputeBounds() {
    // Ignore long/lat angles here.
    return Bounds(size_);
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
