#include "Math/Spin.h"

#include "Util/String.h"

std::string Spin::ToString() const {
    std::string s = "SP[ C=" + Util::ToString(center) +
        " D=" + Util::ToString(axis) +
        " A=" + Util::ToString(angle);
    if (offset != 0)
        s += " OF=" + Util::ToString(offset);
    s += " ]";
    return s;
}
