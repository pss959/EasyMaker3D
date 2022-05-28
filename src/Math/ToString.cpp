#include "Math/ToString.h"

#include "Math/Linear.h"
#include "Util/String.h"

namespace Math {

std::string ToString(float f, float precision) {
    return Util::ToString(RoundToPrecision(f, precision));
}

std::string ToString(const ion::math::VectorBase<2, float> &v, float precision) {
    return "[" +
        ToString(v[0], precision) + ", " +
        ToString(v[1], precision) + "]";
}

std::string ToString(const ion::math::VectorBase<3, float> &v, float precision) {
    return "[" +
        ToString(v[0], precision) + ", " +
        ToString(v[1], precision) + ", " +
        ToString(v[2], precision) + "]";
}

}  // namespace Math
