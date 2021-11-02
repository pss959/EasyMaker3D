#include "SG/ShaderSource.h"

namespace SG {

void ShaderSource::AddFields() {
    AddField(path_);
    Object::AddFields();
}

}  // namespace SG
