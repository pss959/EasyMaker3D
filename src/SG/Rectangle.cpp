#include "SG/Rectangle.h"

#include "SG/SpecBuilder.h"

namespace SG {

std::vector<NParser::FieldSpec> Rectangle::GetFieldSpecs() {
    typedef ion::gfxutils::PlanarShapeSpec::PlaneNormal PlaneNormal_;
    SG::SpecBuilder<Rectangle> builder;
    builder.AddVector2f("size", &Rectangle::size_);
    builder.AddEnum<PlaneNormal_>("plane_normal", &Rectangle::plane_normal_);
    return builder.GetSpecs();
}

}  // namespace SG
