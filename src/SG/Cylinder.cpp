#include "SG/Cylinder.h"

#include <algorithm>

#include <ion/gfxutils/shapeutils.h>

#include "SG/SpecBuilder.h"

namespace SG {

Bounds Cylinder::ComputeBounds() const {
    const float max_diameter = 2.f * std::max(bottom_radius_, top_radius_);
    return Bounds(Vector3f(max_diameter, height_, max_diameter));
}

bool Cylinder::IntersectRay(const Ray &ray, Hit &hit) const {
    return false; // XXXX
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
    return ion::gfxutils::BuildCylinderShape(spec);
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
