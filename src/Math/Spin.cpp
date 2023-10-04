#include "Math/Spin.h"

#include "Util/String.h"

// LCOV_EXCL_START [debug only]
Str Spin::ToString() const {
    Str s = "SP[ C=" + Util::ToString(center) +
        " D=" + Util::ToString(axis) +
        " A=" + Util::ToString(angle);
    if (offset != 0)
        s += " OF=" + Util::ToString(offset);
    s += " ]";
    return s;
}
// LCOV_EXCL_STOP
