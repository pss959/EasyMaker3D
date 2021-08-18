#include "SG/UniformDef.h"

#include "SG/SpecBuilder.h"

namespace SG {

NParser::ObjectSpec UniformDef::GetObjectSpec() {
    SG::SpecBuilder<UniformDef> builder;
    builder.AddEnum<ValueType>("value_type", &UniformDef::value_type_);
    return NParser::ObjectSpec{
        "UniformDef", []{ return new UniformDef; }, builder.GetSpecs() };
}

}  // namespace SG
