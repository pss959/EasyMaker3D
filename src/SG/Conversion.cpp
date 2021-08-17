#include "SG/Conversion.h"

#include <assert.h>

namespace SG {

// Helpers.

typedef std::vector<NParser::Value> Values_;  // Shorthand.

template <int COUNT, typename T>
static void Validate_(const Values_ &vals) {
    assert(vals.size() == COUNT);
    assert(std::holds_alternative<T>(vals[0]));
}

template <int DIM, typename T>
static ion::math::Vector<DIM, T> ConvertVec_(const Values_ &vals) {
    Validate_<DIM, T>(vals);
    ion::math::Vector<DIM, T> vec;
    for (int i; i < DIM; ++i)
        vec[i] = std::get<T>(vals[i]);
    return vec;
}

template <int DIM, typename T>
static ion::math::Matrix<DIM, T> ConvertMatrix_(const Values_ &vals) {
    Validate_<DIM * DIM, T>(vals);
    ion::math::Matrix<DIM, T> m;
    for (int i; i < DIM; ++i)
        for (int j; j < DIM; ++j)
            m[i][j] = std::get<T>(vals[DIM * i + j]);
    return m;
}

Anglef Conversion::ToAnglef(const Values_ &vals) {
    Validate_<1, float>(vals);
    return Anglef::FromDegrees(std::get<float>(vals[0]));
}

Vector2f Conversion::ToVector2f(const Values_ &vals) {
    return ConvertVec_<2, float>(vals);
}

Vector3f Conversion::ToVector3f(const Values_ &vals) {
    return ConvertVec_<3, float>(vals);
}

Vector4f Conversion::ToVector4f(const Values_ &vals) {
    return ConvertVec_<4, float>(vals);
}

Vector2i Conversion::ToVector2i(const Values_ &vals) {
    return ConvertVec_<2, int>(vals);
}

Vector3i Conversion::ToVector3i(const Values_ &vals) {
    return ConvertVec_<3, int>(vals);
}

Vector4i Conversion::ToVector4i(const Values_ &vals) {
    return ConvertVec_<4, int>(vals);
}

Vector2ui Conversion::ToVector2ui(const Values_ &vals) {
    return ConvertVec_<2, unsigned int>(vals);
}

Vector3ui Conversion::ToVector3ui(const Values_ &vals) {
    return ConvertVec_<3, unsigned int>(vals);
}

Vector4ui Conversion::ToVector4ui(const Values_ &vals) {
    return ConvertVec_<4, unsigned int>(vals);
}

Rotationf Conversion::ToRotationf(const Values_ &vals) {
    const Vector4f vec = ToVector4f(vals);
    return Rotationf::FromAxisAndAngle(Vector3f(vec[0], vec[1], vec[2]),
                                       Anglef::FromDegrees(vec[3]));
}

Matrix2f Conversion::ToMatrix2f(const Values_ &vals) {
    return ConvertMatrix_<2, float>(vals);
}

Matrix3f Conversion::ToMatrix3f(const Values_ &vals) {
    return ConvertMatrix_<3, float>(vals);
}

Matrix4f Conversion::ToMatrix4f(const Values_ &vals) {
    return ConvertMatrix_<4, float>(vals);
}

}  // namespace SG
