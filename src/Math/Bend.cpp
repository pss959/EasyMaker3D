#include "Math/Bend.h"

#include "Util/String.h"

std::string Bend::ToString() const {
    return "TW [ C=" + Util::ToString(center) +
        " AX=" + Util::ToString(axis) +
        " A=" + Util::ToString(angle) + "]";
}
