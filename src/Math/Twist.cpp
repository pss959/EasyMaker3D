#include "Math/Twist.h"

#include "Util/String.h"

std::string Twist::ToString() const {
    return "TW [ C=" + Util::ToString(center) +
        " AX=" + Util::ToString(axis) +
        " A=" + Util::ToString(angle) + "]";
}
