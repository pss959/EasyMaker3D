//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/UniformDef.h"

namespace SG {

void UniformDef::AddFields() {
    AddField(value_type_.Init("value_type", ValueType::kFloatUniform));

    Object::AddFields();
}

}  // namespace SG
