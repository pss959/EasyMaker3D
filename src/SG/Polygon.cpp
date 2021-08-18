#include "SG/Polygon.h"

#include "SG/SpecBuilder.h"

namespace SG {

NParser::ObjectSpec Polygon::GetObjectSpec() {
    typedef ion::gfxutils::PlanarShapeSpec::PlaneNormal PlaneNormal_;
    SG::SpecBuilder<Polygon> builder;
    builder.AddInt("sides", &Polygon::sides_);
    builder.AddEnum<PlaneNormal_>("plane_normal", &Polygon::plane_normal_);
    return NParser::ObjectSpec{
        "Polygon", false, []{ return new Polygon; }, builder.GetSpecs() };
}

}  // namespace SG
