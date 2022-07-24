#include "SG/NamedColor.h"

#include "Util/Assert.h"

namespace SG {

void NamedColor::AddFields() {
    AddField(color_.Init("color"));

    Object::AddFields();
}

}  // namespace SG
