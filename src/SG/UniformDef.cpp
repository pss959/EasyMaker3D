#include "SG/UniformDef.h"

#include "SG/SpecBuilder.h"

namespace SG {

void UniformDef::SetUpIon(IonContext &context) {
    // No harm always doing this.
    spec_.name       = GetName();
    spec_.value_type = value_type_;
}

Parser::ObjectSpec UniformDef::GetObjectSpec() {
    SG::SpecBuilder<UniformDef> builder;
    builder.AddInt("count",                  &UniformDef::count_);
    builder.AddEnum<ValueType>("value_type", &UniformDef::value_type_);
    return Parser::ObjectSpec{
        "UniformDef", true, []{ return new UniformDef; }, builder.GetSpecs() };
}

}  // namespace SG
