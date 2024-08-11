//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/Types.h"

#include "Math/ToString.h"

// ToStr() specializations.

// LCOV_EXCL_START [debug only]
template <> Str ToStr(const float &f) {
    return Math::ToString(f, .001f);
}

template <> Str ToStr(const Point3f &t) {
    return Math::ToString(t, .001f);
}

template <> Str ToStr(const Vector3f &t) {
    return Math::ToString(t, .001f);
}

template <> Str ToStr(const Anglef &a) {
    return Math::ToString(a.Degrees(), .1f);
}

template <> Str ToStr(const Rotationf &r) {
    Vector3f axis;
    Anglef   angle;
    r.GetAxisAndAngle(&axis, &angle);
    return "R[" + ToStr(axis) + ", " + ToStr(angle) + "]";
}
// LCOV_EXCL_STOP
