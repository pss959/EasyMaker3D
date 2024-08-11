//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/Gantry.h"

namespace SG {

void Gantry::AddFields() {
    AddField(cameras_.Init("cameras"));

    Object::AddFields();
}

void Gantry::SetHeight(float height) {
    height_ = height;

    // Update all of the cameras.
    for (auto &cam: GetCameras())
        cam->SetHeight(height);
}

}  // namespace SG
