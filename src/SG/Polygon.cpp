#include "SG/Polygon.h"

#include "SG/SpecBuilder.h"

namespace SG {

NParser::ObjectSpec Polygon::GetObjectSpec() {
    SG::SpecBuilder<Polygon> builder;
    builder.AddInt("sides", &Polygon::sides_);
    builder.AddEnum<PlaneNormal>("plane_normal", &Polygon::plane_normal_);
    return NParser::ObjectSpec{
        "Polygon", false, []{ return new Polygon; }, builder.GetSpecs() };
}

}  // namespace SG
