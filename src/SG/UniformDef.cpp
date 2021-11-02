#include "SG/UniformDef.h"

namespace SG {

void UniformDef::AddFields() {
    AddField(value_type_);
    Object::AddFields();
}

}  // namespace SG
