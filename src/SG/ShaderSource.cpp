#include "SG/ShaderSource.h"

namespace SG {

void ShaderSource::AddFields() {
    AddField(path_.Init("path"));

    Object::AddFields();
}

}  // namespace SG
