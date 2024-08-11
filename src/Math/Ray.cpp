//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/Ray.h"

Str Ray::ToString() const {
    return "RAY [o=" + ToStr(origin) + " d=" + ToStr(direction) + "]";
}
