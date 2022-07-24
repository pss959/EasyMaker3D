#include "SG/UniformDef.h"

namespace SG {

void UniformDef::AddFields() {
    AddField(value_type_.Init("value_type", ValueType::kFloatUniform));

    Object::AddFields();
}

}  // namespace SG
