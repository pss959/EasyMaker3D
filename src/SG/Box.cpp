#include "SG/Box.h"

#include <ion/gfxutils/shapeutils.h>

#include "SG/SpecBuilder.h"

namespace SG {

ion::gfx::ShapePtr Box::CreateIonShape() {
    ion::gfxutils::BoxSpec spec;
    spec.size = size_;
    return ion::gfxutils::BuildBoxShape(spec);
}

NParser::ObjectSpec Box::GetObjectSpec() {
    SG::SpecBuilder<Box> builder;
    builder.AddVector3f("size", &Box::size_);
    return NParser::ObjectSpec{
        "Box", false, []{ return new Box; }, builder.GetSpecs() };
}

}  // namespace SG
