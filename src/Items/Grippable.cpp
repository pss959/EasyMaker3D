//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Items/Grippable.h"

#include <ion/math/angleutils.h>

#include "Items/Controller.h"
#include "Util/Assert.h"

const SG::NodePath & Grippable::GetPath() const {
    ASSERT(! path_.empty());
    return path_;
}

int Grippable::GetBestDirIndex(const std::vector<Vector3f> &candidates,
                               const Vector3f &dir, const Anglef &max_angle,
                               bool &is_opposite) {
    Anglef smallest_angle = Anglef::FromDegrees(360);
    int    smallest_index = -1;
    for (size_t i = 0; i < candidates.size(); ++i) {
        const auto &candidate = candidates[i];
        const Anglef angle0 = ion::math::AngleBetween(candidate,  dir);
        const Anglef angle1 = ion::math::AngleBetween(candidate, -dir);
        if (angle0 < smallest_angle) {
            smallest_angle = angle0;
            smallest_index = i;
            is_opposite    = false;
        }
        if (angle1 < smallest_angle) {
            smallest_angle = angle1;
            smallest_index = i;
            is_opposite    = true;
        }
    }
    if (max_angle.Radians() > 0 && smallest_angle > max_angle)
        smallest_index = -1;
    return smallest_index;
}

int Grippable::GetBestAxis(const Vector3f &dir, const Anglef &max_angle,
                           bool &is_opposite) {
    static std::vector<Vector3f> s_axes{
        Vector3f::AxisX(),
        Vector3f::AxisY(),
        Vector3f::AxisZ()
    };
    return GetBestDirIndex(s_axes, dir, max_angle, is_opposite);
}
