#include "SG/UniformDef.h"

#include "SG/SpecBuilder.h"

namespace SG {

std::vector<NParser::FieldSpec> UniformDef::GetFieldSpecs() {
    SG::SpecBuilder<UniformDef> builder;
    builder.AddEnum<ValueType>("value_type", &UniformDef::value_type_);
    return builder.GetSpecs();
}

}  // namespace SG
