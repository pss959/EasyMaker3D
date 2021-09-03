#include "Parser/Field.h"

#include "Math/Types.h"

namespace Parser {

// ----------------------------------------------------------------------------
// Instantiate TField::ParseValue() function for supported basic types.
// ----------------------------------------------------------------------------

template <> void TField<bool>::ParseValue(Scanner &scanner) {
    value_ = scanner.ScanBool();
}

template <> void TField<int>::ParseValue(Scanner &scanner) {
    value_ = scanner.ScanInteger();
}

template <> void TField<unsigned int>::ParseValue(Scanner &scanner) {
    value_ = scanner.ScanUInteger();
}

template <> void TField<std::string>::ParseValue(Scanner &scanner) {
    value_ = scanner.ScanQuotedString();
}

template <> void TField<float>::ParseValue(Scanner &scanner) {
    value_ = scanner.ScanFloat();
}

// ----------------------------------------------------------------------------
// Instantiate TField::ParseValue() function for supported Math vector types.
// Note that the order of calls to Scanner must be explicit.
// ----------------------------------------------------------------------------

#define PARSE_VEC_(TYPE, DIM, SCAN_FUNC)                                \
template <> void TField<TYPE>::ParseValue(Scanner &scanner) {           \
    for (int i = 0; i < DIM; ++i)                                       \
        value_[i] = scanner.SCAN_FUNC();                                \
}

PARSE_VEC_(Vector2f, 2, ScanFloat)
PARSE_VEC_(Vector3f, 3, ScanFloat)
PARSE_VEC_(Vector4f, 4, ScanFloat)

PARSE_VEC_(Vector2i, 2, ScanInteger)
PARSE_VEC_(Vector3i, 3, ScanInteger)
PARSE_VEC_(Vector4i, 4, ScanInteger)

PARSE_VEC_(Vector2ui, 2, ScanUInteger)
PARSE_VEC_(Vector3ui, 3, ScanUInteger)
PARSE_VEC_(Vector4ui, 4, ScanUInteger)

PARSE_VEC_(Point2f, 2, ScanFloat)
PARSE_VEC_(Point3f, 3, ScanFloat)
PARSE_VEC_(Point2i, 2, ScanInteger)

#undef PARSE_VEC_

// ----------------------------------------------------------------------------
// Instantiate TField::ParseValue() function for other Math types.
// ----------------------------------------------------------------------------

template <> void TField<Rotationf>::ParseValue(Scanner &scanner) {
    Vector4f vec;
    for (int i = 0; i < 4; ++i)
        vec[i] = scanner.ScanFloat();
    value_ = Rotationf::FromAxisAndAngle(Vector3f(vec[0], vec[1], vec[2]),
                                         Anglef::FromDegrees(vec[3]));
}

#define PARSE_MATRIX_(TYPE, DIM)                                        \
template <> void TField<TYPE>::ParseValue(Scanner &scanner) {           \
    for (int i = 0; i < DIM; ++i)                                       \
        for (int j; j < DIM; ++j)                                       \
            value_[i][j] = scanner.ScanFloat();                         \
}

PARSE_MATRIX_(Matrix2f, 2)
PARSE_MATRIX_(Matrix3f, 3)
PARSE_MATRIX_(Matrix4f, 4)

#undef PARSE_MATRIX_

}  // namespace Parser
