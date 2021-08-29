#include "SG/Box.h"

#include <ion/gfxutils/shapeutils.h>

#include "SG/SpecBuilder.h"

namespace SG {

Bounds Box::ComputeBounds() {
    return Bounds(size_);
}

ion::gfx::ShapePtr Box::CreateIonShape() {
    ion::gfxutils::BoxSpec spec;
    spec.size = size_;
    return ion::gfxutils::BuildBoxShape(spec);
}

Parser::ObjectSpec Box::GetObjectSpec() {
    SG::SpecBuilder<Box> builder;
    builder.AddVector3f("size", &Box::size_);
    return Parser::ObjectSpec{
        "Box", false, []{ return new Box; }, builder.GetSpecs() };
}

}  // namespace SG
