//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/SubImage.h"

namespace SG {

void SubImage::AddFields() {
    AddField(texture_scale_.Init("texture_scale",   Vector2f(1, 1)));
    AddField(texture_offset_.Init("texture_offset", Vector2f(0, 0)));

    Object::AddFields();
}

}  // namespace SG
