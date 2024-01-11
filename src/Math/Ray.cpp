#include "Math/Ray.h"

Str Ray::ToString() const {
    return "RAY [o=" + ToStr(origin) + " d=" + ToStr(direction) + "]";
}
