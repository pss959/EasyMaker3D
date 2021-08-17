#include "SG/Polygon.h"

#include "SG/SpecBuilder.h"

namespace SG {

std::vector<NParser::FieldSpec> Polygon::GetFieldSpecs() {
    typedef ion::gfxutils::PlanarShapeSpec::PlaneNormal PlaneNormal_;
    SG::SpecBuilder<Polygon> builder;
    builder.AddInt("sides", &Polygon::sides_);
    builder.AddEnum<PlaneNormal_>("plane_normal", &Polygon::plane_normal_);
    return builder.GetSpecs();
}

}  // namespace SG
