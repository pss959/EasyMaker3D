#include "SG/Cylinder.h"

#include <ion/gfxutils/shapeutils.h>

#include "SG/SpecBuilder.h"

namespace SG {

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
    return ion::gfxutils::BuildCylinderShape(spec);
}

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
        "Cylinder", false, []{ return new Cylinder; }, builder.GetSpecs() };
}

}  // namespace SG
