#include "Parser/Field.h"

#include "Math/Curves.h"
#include "Math/Types.h"
#include "Parser/Object.h"

namespace Parser {

// ----------------------------------------------------------------------------
// General Field functions.
// ----------------------------------------------------------------------------

void Field::ThrowObjectTypeError(Scanner &scanner, const ObjectPtr &obj) {
    // This requires knowledge of Object internals, so it cannot be in the
    // header file. It is in the Field class because Field is not templated.
    scanner.Throw("Incorrect object type (" + obj->GetTypeName()
                  + ") found in field " + GetName());
}

// ----------------------------------------------------------------------------
// Instantiate Field::ScanValue() function for supported basic types.
// ----------------------------------------------------------------------------

template <> bool Field::ScanValue<bool>(Scanner &scanner) {
    return scanner.ScanBool();
}

template <> int Field::ScanValue<int>(Scanner &scanner) {
    return scanner.ScanInteger();
}

template <> unsigned int Field::ScanValue<unsigned int>(Scanner &scanner) {
    return scanner.ScanUInteger();
}

template <> size_t Field::ScanValue<size_t>(Scanner &scanner) {
    return scanner.ScanUInteger();
}

template <> std::string Field::ScanValue<std::string>(Scanner &scanner) {
    return scanner.ScanQuotedString();
}

template <> float Field::ScanValue<float>(Scanner &scanner) {
    return scanner.ScanFloat();
}

// ----------------------------------------------------------------------------
// Instantiate Field::ScanValue() function for supported Math vector types.
// Note that the order of calls to Scanner must be explicit.
// ----------------------------------------------------------------------------

#define SCAN_VEC_(TYPE, DIM, SCAN_FUNC)                                 \
template <> TYPE Field::ScanValue<TYPE>(Scanner &scanner) {             \
    TYPE vec;                                                           \
    for (int i = 0; i < DIM; ++i)                                       \
        vec[i] = scanner.SCAN_FUNC();                                   \
    return vec;                                                         \
}

SCAN_VEC_(Vector2f, 2, ScanFloat)
SCAN_VEC_(Vector3f, 3, ScanFloat)
SCAN_VEC_(Vector4f, 4, ScanFloat)

SCAN_VEC_(Vector2i, 2, ScanInteger)
SCAN_VEC_(Vector3i, 3, ScanInteger)
SCAN_VEC_(Vector4i, 4, ScanInteger)

SCAN_VEC_(Vector2ui, 2, ScanUInteger)
SCAN_VEC_(Vector3ui, 3, ScanUInteger)
SCAN_VEC_(Vector4ui, 4, ScanUInteger)

SCAN_VEC_(Point2f, 2, ScanFloat)
SCAN_VEC_(Point3f, 3, ScanFloat)
SCAN_VEC_(Point2i, 2, ScanInteger)

#undef SCAN_VEC_

// ----------------------------------------------------------------------------
// Instantiate Field::ScanValue() function for supported Math matrix types.
// Note that the order of calls to Scanner must be explicit.
// ----------------------------------------------------------------------------

#define SCAN_MATRIX_(TYPE, DIM)                                         \
template <> TYPE Field::ScanValue<TYPE>(Scanner &scanner) {             \
    TYPE m;                                                             \
    for (int i = 0; i < DIM; ++i)                                       \
        for (int j = 0; j < DIM; ++j)                                   \
            m[i][j] = scanner.ScanFloat();                              \
    return m;                                                           \
}

SCAN_MATRIX_(Matrix2f, 2)
SCAN_MATRIX_(Matrix3f, 3)
SCAN_MATRIX_(Matrix4f, 4)

#undef SCAN_MATRIX_

// ----------------------------------------------------------------------------
// Instantiate Field::ScanValue() function for other Math types.
// ----------------------------------------------------------------------------

template <> Color Field::ScanValue<Color>(Scanner &scanner) {
    return scanner.ScanColor();
}

template <> Anglef Field::ScanValue<Anglef>(Scanner &scanner) {
    return Anglef::FromDegrees(scanner.ScanFloat());
}

template <> CircleArc Field::ScanValue<CircleArc>(Scanner &scanner) {
    const Anglef start_angle = ScanValue<Anglef>(scanner);
    const Anglef arc_angle   = ScanValue<Anglef>(scanner);
    return CircleArc(start_angle, arc_angle);
}

template <> Rotationf Field::ScanValue<Rotationf>(Scanner &scanner) {
    const Vector4f vec = ScanValue<Vector4f>(scanner);
    return Rotationf::FromAxisAndAngle(Vector3f(vec[0], vec[1], vec[2]),
                                       Anglef::FromDegrees(vec[3]));
}

template <> Plane Field::ScanValue<Plane>(Scanner &scanner) {
    const Vector3f normal   = ScanValue<Vector3f>(scanner);
    const float    distance = scanner.ScanFloat();
    return Plane(distance, normal);
}

// ----------------------------------------------------------------------------
// Implements scanning an std::vector of values.
// ----------------------------------------------------------------------------

void Field::ScanValues(Scanner &scanner,
                       const std::function<void()> &scan_func) {
    // Values are contained within square brackets and are separated by commas.
    scanner.ScanExpectedChar('[');
    while (true) {
        if (scanner.PeekChar() == ']')
            break;
        scan_func();
        if (scanner.PeekChar() == ',')
            scanner.ScanExpectedChar(',');
    }
    scanner.ScanExpectedChar(']');
}

}  // namespace Parser
