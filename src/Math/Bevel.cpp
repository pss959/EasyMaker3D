#include "Math/Bevel.h"

#include "Util/String.h"

std::string Bevel::ToString() const {
    return "BEV [ " + profile.ToString() +
        " SC=" + Util::ToString(scale) +
        " MA=" + Util::ToString(max_angle) + "]";
}
