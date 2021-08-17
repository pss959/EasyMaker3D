#include "SG/Box.h"

#include "SG/SpecBuilder.h"

namespace SG {

std::vector<NParser::FieldSpec> Box::GetFieldSpecs() {
    SG::SpecBuilder<Box> builder;
    builder.AddVector3f("size", &Box::size_);
    return builder.GetSpecs();
}

}  // namespace SG
