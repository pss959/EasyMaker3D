#include "SG/Material.h"

namespace SG {

void Material::AddFields() {
    AddField(base_color_);
    AddField(emissive_color_);
    AddField(smoothness_);
    AddField(metalness_);
    Object::AddFields();
}

}  // namespace SG
