//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/ShaderSource.h"

namespace SG {

void ShaderSource::AddFields() {
    AddField(path_.Init("path"));

    Object::AddFields();
}

}  // namespace SG
