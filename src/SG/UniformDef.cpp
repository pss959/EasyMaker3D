#include "SG/UniformDef.h"

namespace SG {

void UniformDef::AddFields() {
    AddField(value_type_);
}

void UniformDef::SetUpIon(IonContext &context) {
    // No harm always doing this.
    spec_.name       = GetName();
    spec_.value_type = value_type_;
}

}  // namespace SG
