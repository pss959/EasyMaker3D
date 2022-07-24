#include "SG/PointLight.h"

namespace SG {

void PointLight::AddFields() {
    AddField(position_.Init("position",         Point3f::Zero()));
    AddField(color_.Init("color",               Color::White()));
    AddField(cast_shadows_.Init("cast_shadows", true));

    Object::AddFields();
}

}  // namespace SG
