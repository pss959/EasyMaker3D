#include "Parser/ValueWriter.h"

#include "Math/Types.h"

namespace Parser {

// ----------------------------------------------------------------------------
// Instantiate ValueWriter::WriteValue() for supported basic types.
// ----------------------------------------------------------------------------

template <> void ValueWriter::WriteValue(const bool &value) {
    out_ << (value ? "True" : "False");
}

template <> void ValueWriter::WriteValue(const int &value) {
    out_ << value;
}

template <> void ValueWriter::WriteValue(const unsigned int &value) {
    out_ << value;
}

template <> void ValueWriter::WriteValue(const float &value) {
    out_ << value;
}

template <> void ValueWriter::WriteValue(const std::string &value) {
    out_ << '"' << value << '"';
}

// ----------------------------------------------------------------------------
// Instantiate ValueWriter::WriteValue() for supported Math vector types.
// ----------------------------------------------------------------------------

#define WRITE_VEC_(TYPE, DIM)                                           \
template <> void ValueWriter::WriteValue(const TYPE &vec) {             \
    for (int i = 0; i < DIM; ++i) {                                     \
        if (i > 0)                                                      \
            out_ << ' ';                                                \
        out_ << vec[i];                                                 \
    }                                                                   \
}

WRITE_VEC_(Vector2f, 2)
WRITE_VEC_(Vector3f, 3)
WRITE_VEC_(Vector4f, 4)

WRITE_VEC_(Vector2i, 2)
WRITE_VEC_(Vector3i, 3)
WRITE_VEC_(Vector4i, 4)

WRITE_VEC_(Vector2ui, 2)
WRITE_VEC_(Vector3ui, 3)
WRITE_VEC_(Vector4ui, 4)

WRITE_VEC_(Point2f, 2)
WRITE_VEC_(Point3f, 3)
WRITE_VEC_(Point2i, 2)

#undef WRITE_VEC_

// ----------------------------------------------------------------------------
// Instantiate ValueWriter::WriteValue() for other Math types.
// ----------------------------------------------------------------------------

template <> void ValueWriter::WriteValue(const Anglef &value) {
    out_ << value.Degrees();
}

template <> void ValueWriter::WriteValue(const Rotationf &value) {
    Vector3f axis;
    Anglef   angle;
    value.GetAxisAndAngle(&axis, &angle);
    WriteValue(axis);
    out_ << ' ';
    WriteValue(angle);
}

#define WRITE_MATRIX_(TYPE, DIM)                                        \
template <> void ValueWriter::WriteValue(const TYPE &value) {           \
    for (int i = 0; i < DIM; ++i) {                                     \
        for (int j; j < DIM; ++j) {                                     \
            if (i > 0 || j == 0)                                        \
                out_ << ' ';                                            \
            out_ << value[i][j];                                        \
        }                                                               \
    }                                                                   \
}

WRITE_MATRIX_(Matrix2f, 2)
WRITE_MATRIX_(Matrix3f, 3)
WRITE_MATRIX_(Matrix4f, 4)

#undef WRITE_MATRIX_


}  // namespace Parser
