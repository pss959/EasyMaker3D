#include "SG/Rectangle.h"

#include "SG/SpecBuilder.h"

namespace SG {

NParser::ObjectSpec Rectangle::GetObjectSpec() {
    typedef ion::gfxutils::PlanarShapeSpec::PlaneNormal PlaneNormal_;
    SG::SpecBuilder<Rectangle> builder;
    builder.AddVector2f("size", &Rectangle::size_);
    builder.AddEnum<PlaneNormal_>("plane_normal", &Rectangle::plane_normal_);
    return NParser::ObjectSpec{
        "Rectangle", []{ return new Rectangle; }, builder.GetSpecs() };
}

}  // namespace SG
