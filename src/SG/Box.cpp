#include "SG/Box.h"

#include "SG/SpecBuilder.h"

namespace SG {

NParser::ObjectSpec Box::GetObjectSpec() {
    SG::SpecBuilder<Box> builder;
    builder.AddVector3f("size", &Box::size_);
    return NParser::ObjectSpec{
        "Box", false, []{ return new Box; }, builder.GetSpecs() };
}

}  // namespace SG
