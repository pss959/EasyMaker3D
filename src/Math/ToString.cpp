#include "Math/ToString.h"

#include <iomanip>
#include <sstream>

#include "Math/Linear.h"
#include "Util/String.h"

namespace Math {

Str ToString(float f, float precision) {
    return Util::ToString(RoundToPrecision(f, precision));
}

Str ToString(const VectorBase<2, float> &v, float precision) {
    return "[" +
        ToString(v[0], precision) + ", " +
        ToString(v[1], precision) + "]";
}

Str ToString(const VectorBase<3, float> &v, float precision) {
    return "[" +
        ToString(v[0], precision) + ", " +
        ToString(v[1], precision) + ", " +
        ToString(v[2], precision) + "]";
}

Str ToString(const Rotationf &r) {
    Vector3f axis;
    Anglef   angle;
    r.GetAxisAndAngle(&axis, &angle);
    return ToString(angle.Degrees(), .1f) + "d / " + ToString(axis, .001f);
}

Str ToString(const Matrix4f &m, float precision) {
    std::ostringstream out;
    for (int row = 0; row < 4; ++row) {
        out << (row == 0 ? "[" : " ");
        for (int col = 0; col < 4; ++col)
            out << ' ' << std::setw(6) << ToString(m[row][col], precision);
        out << (row == 3 ? " ]\n" : "\n");
    }
    return out.str();
}

}  // namespace Math
