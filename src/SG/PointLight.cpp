#include "SG/PointLight.h"

namespace SG {

void PointLight::AddFields() {
    AddField(position_);
    AddField(color_);
    AddField(cast_shadows_);
}

}  // namespace SG
