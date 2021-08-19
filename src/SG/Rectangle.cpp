#include "SG/Rectangle.h"

#include "SG/SpecBuilder.h"

namespace SG {

NParser::ObjectSpec Rectangle::GetObjectSpec() {
    SG::SpecBuilder<Rectangle> builder;
    builder.AddVector2f("size", &Rectangle::size_);
    builder.AddEnum<PlaneNormal>("plane_normal", &Rectangle::plane_normal_);
    return NParser::ObjectSpec{
        "Rectangle", false, []{ return new Rectangle; }, builder.GetSpecs() };
}

}  // namespace SG
