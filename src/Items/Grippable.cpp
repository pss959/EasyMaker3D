#include "Items/Grippable.h"

#include <ion/math/angleutils.h>

#include "Items/Controller.h"
#include "Util/Assert.h"

const SG::NodePath & Grippable::GetPath() const {
    ASSERT(! path_.empty());
    return path_;
}

size_t Grippable::GetBestDirChoice(const std::vector<DirChoice> &choices,
                                   const Vector3f &direction,
                                   const Anglef &max_angle) {
    Anglef smallest_angle = Anglef::FromDegrees(360);
    size_t smallest_index = ion::base::kInvalidIndex;
    for (size_t i = 0; i < choices.size(); ++i) {
        const Anglef angle =
            ion::math::AngleBetween(choices[i].direction, direction);
        if (angle < smallest_angle) {
            smallest_angle = angle;
            smallest_index = i;
        }
    }
    if (max_angle.Radians() > 0 && smallest_angle > max_angle)
        smallest_index = ion::base::kInvalidIndex;
    return smallest_index;
}

size_t Grippable::GetBestDirChoiceSymmetric(
    const std::vector<DirChoice> &choices,
    const Vector3f &direction, const Anglef &max_angle, bool &is_opposite) {
    Anglef smallest_angle = Anglef::FromDegrees(360);
    size_t smallest_index = ion::base::kInvalidIndex;
    for (size_t i = 0; i < choices.size(); ++i) {
        const Anglef angle =
            ion::math::AngleBetween(choices[i].direction, direction);
        if (angle < smallest_angle) {
            smallest_angle = angle;
            smallest_index = i;
            is_opposite    = false;
        }
    }
    for (size_t i = 0; i < choices.size(); ++i) {
        const Anglef angle =
            ion::math::AngleBetween(choices[i].direction, -direction);
        if (angle < smallest_angle) {
            smallest_angle = angle;
            smallest_index = i;
            is_opposite    = true;
        }
    }
    if (max_angle.Radians() > 0 && smallest_angle > max_angle)
        smallest_index = ion::base::kInvalidIndex;
    return smallest_index;
}
