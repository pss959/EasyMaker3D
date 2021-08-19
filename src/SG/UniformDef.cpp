#include "SG/UniformDef.h"

#include "SG/SpecBuilder.h"

namespace SG {

void UniformDef::SetUpIon(IonContext &context) {
    // No harm always doing this.
    spec_.name       = GetName();
    spec_.value_type = value_type_;
}

NParser::ObjectSpec UniformDef::GetObjectSpec() {
    SG::SpecBuilder<UniformDef> builder;
    builder.AddEnum<ValueType>("value_type", &UniformDef::value_type_);
    return NParser::ObjectSpec{
        "UniformDef", true, []{ return new UniformDef; }, builder.GetSpecs() };
}

}  // namespace SG
