#include "SG/Cylinder.h"

#include "SG/SpecBuilder.h"

namespace SG {

NParser::ObjectSpec Cylinder::GetObjectSpec() {
    SG::SpecBuilder<Cylinder> builder;
    builder.AddFloat("bottom_radius",  &Cylinder::bottom_radius_);
    builder.AddFloat("top_radius",     &Cylinder::top_radius_);
    builder.AddFloat("height",         &Cylinder::height_);
    builder.AddBool("has_top_cap",     &Cylinder::has_top_cap_);
    builder.AddBool("has_bottom_cap",  &Cylinder::has_bottom_cap_);
    builder.AddInt("shaft_band_count", &Cylinder::shaft_band_count_);
    builder.AddInt("cap_band_count",   &Cylinder::cap_band_count_);
    builder.AddInt("sector_count",     &Cylinder::sector_count_);
    return NParser::ObjectSpec{
        "Cylinder", []{ return new Cylinder; }, builder.GetSpecs() };
}

}  // namespace SG
