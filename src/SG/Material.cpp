//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/Material.h"

namespace SG {

void Material::AddFields() {
    AddField(base_color_.Init("base_color",         Color::White()));
    AddField(emissive_color_.Init("emissive_color", Color::Clear()));
    AddField(smoothness_.Init("smoothness",         0));
    AddField(metalness_.Init("metalness",           0));

    Object::AddFields();
}

}  // namespace SG
