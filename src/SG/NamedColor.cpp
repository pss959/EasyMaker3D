//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/NamedColor.h"

#include "Util/Assert.h"

namespace SG {

void NamedColor::AddFields() {
    AddField(color_.Init("color"));

    Object::AddFields();
}

}  // namespace SG
