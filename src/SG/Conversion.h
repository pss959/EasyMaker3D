#pragma once

#include <assert.h>

#include <string>
#include <vector>

#include "NParser/Value.h"
#include "NParser/ValueType.h"
#include "SG/Math.h"
#include "Util/Enum.h"

//! This class contains convenient static functions to convert NParser::Value
//! types to SG/Ion types. Each asserts if used incorrectly.
namespace SG {

class Conversion {
  public:
    typedef std::vector<NParser::Value> Values;  // Shorthand.

    template <typename E>
    static E ToEnum(const Values &vals) {
        E e;
        bool ok = Util::EnumFromString<E>(ToString(vals), e);
        assert(ok);
        return e;
    }

    static std::string ToString(const    Values &vals);
    static Anglef      ToAnglef(const    Values &vals);
    static Vector2f    ToVector2f(const  Values &vals);
    static Vector3f    ToVector3f(const  Values &vals);
    static Vector4f    ToVector4f(const  Values &vals);
    static Vector2i    ToVector2i(const  Values &vals);
    static Vector3i    ToVector3i(const  Values &vals);
    static Vector4i    ToVector4i(const  Values &vals);
    static Vector2ui   ToVector2ui(const Values &vals);
    static Vector3ui   ToVector3ui(const Values &vals);
    static Vector4ui   ToVector4ui(const Values &vals);
    static Rotationf   ToRotationf(const Values &vals);
    static Matrix2f    ToMatrix2f(const  Values &vals);
    static Matrix3f    ToMatrix3f(const  Values &vals);
    static Matrix4f    ToMatrix4f(const  Values &vals);
};

}  // namespace SG
