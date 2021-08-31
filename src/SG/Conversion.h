#pragma once

#include <string>
#include <vector>

#include "Assert.h"
#include "Math/Types.h"
#include "Parser/Value.h"
#include "Parser/ValueType.h"

//! This class contains convenient static functions to convert Parser::Value
//! types to SG/Ion types. Each asserts if used incorrectly.
namespace SG {

class Conversion {
  public:
    typedef std::vector<Parser::Value> Values;  // Shorthand.

    static std::string ToString(const    Values &vals);
    static Anglef      ToAnglef(const    Values &vals);
    static Point2f     ToPoint2f(const   Values &vals);
    static Point3f     ToPoint3f(const   Values &vals);
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

    //! Generic templated conversion function, specialized for each type.
    template <typename T> static T To(const Values &vals);
};

// Specializations.
#define SPECIALIZE_TO_(TYPE, FUNC)                                      \
template <> inline                                                      \
TYPE Conversion::To(const Values &vals) { return FUNC(vals); }

SPECIALIZE_TO_(std::string, ToString)
SPECIALIZE_TO_(Anglef,      ToAnglef)
SPECIALIZE_TO_(Point2f,     ToPoint2f)
SPECIALIZE_TO_(Point3f,     ToPoint3f)
SPECIALIZE_TO_(Vector2f,    ToVector2f)
SPECIALIZE_TO_(Vector3f,    ToVector3f)
SPECIALIZE_TO_(Vector4f,    ToVector4f)
SPECIALIZE_TO_(Vector2i,    ToVector2i)
SPECIALIZE_TO_(Vector3i,    ToVector3i)
SPECIALIZE_TO_(Vector4i,    ToVector4i)
SPECIALIZE_TO_(Vector2ui,   ToVector2ui)
SPECIALIZE_TO_(Vector3ui,   ToVector3ui)
SPECIALIZE_TO_(Vector4ui,   ToVector4ui)
SPECIALIZE_TO_(Rotationf,   ToRotationf)
SPECIALIZE_TO_(Matrix2f,    ToMatrix2f)
SPECIALIZE_TO_(Matrix3f,    ToMatrix3f)
SPECIALIZE_TO_(Matrix4f,    ToMatrix4f)

#undef SPECIALIZE_TO_

}  // namespace SG
