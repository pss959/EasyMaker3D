#include "SG/Ellipsoid.h"

#include "SG/SpecBuilder.h"

namespace SG {

NParser::ObjectSpec Ellipsoid::GetObjectSpec() {
    SG::SpecBuilder<Ellipsoid> builder;
    builder.AddAnglef("longitude_start", &Ellipsoid::longitude_start_);
    builder.AddAnglef("longitude_end",   &Ellipsoid::longitude_end_);
    builder.AddAnglef("latitude_start",  &Ellipsoid::latitude_start_);
    builder.AddAnglef("latitude_end",    &Ellipsoid::latitude_end_);
    builder.AddInt("band_count",         &Ellipsoid::band_count_);
    builder.AddInt("sector_count",       &Ellipsoid::sector_count_);
    builder.AddVector3f("size",          &Ellipsoid::size_);
    return NParser::ObjectSpec{
        "Ellipsoid", []{ return new Ellipsoid; }, builder.GetSpecs() };
}

}  // namespace SG
