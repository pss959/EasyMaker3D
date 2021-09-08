#include "SG/UniformDef.h"

namespace SG {

void UniformDef::AddFields() {
    AddField(value_type_);
}

void UniformDef::SetUpIon(const ContextPtr &context) {
    Object::SetUpIon(context);

    // No harm always doing this.
    spec_.name       = GetName();
    spec_.value_type = value_type_;
}

}  // namespace SG
