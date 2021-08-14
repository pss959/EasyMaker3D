#pragma once

#include <ion/math/angle.h>
#include <ion/math/matrix.h>
#include <ion/math/rotation.h>
#include <ion/math/vector.h>

#include "Parser/Field.h"
#include "Util/Enum.h"

//! This class contains convenient static functions to convert Parser::Value
//! types to Ion types. Each asserts if used incorrectly.
//!
//! \ingroup Input
namespace Input {

class Conversion {
  public:
    static ion::math::Anglef    ToAnglef(const    Parser::Field &field);
    static ion::math::Vector2f  ToVector2f(const  Parser::Field &field);
    static ion::math::Vector3f  ToVector3f(const  Parser::Field &field);
    static ion::math::Vector4f  ToVector4f(const  Parser::Field &field);
    static ion::math::Vector2i  ToVector2i(const  Parser::Field &field);
    static ion::math::Vector3i  ToVector3i(const  Parser::Field &field);
    static ion::math::Vector4i  ToVector4i(const  Parser::Field &field);
    static ion::math::Vector2i  ToVector2ui(const Parser::Field &field);
    static ion::math::Vector3i  ToVector3ui(const Parser::Field &field);
    static ion::math::Vector4i  ToVector4ui(const Parser::Field &field);
    static ion::math::Rotationf ToRotationf(const Parser::Field &field);
    static ion::math::Matrix2f  ToMatrix2f(const  Parser::Field &field);
    static ion::math::Matrix3f  ToMatrix3f(const  Parser::Field &field);
    static ion::math::Matrix4f  ToMatrix4f(const  Parser::Field &field);

    template <typename E>
    static bool ToEnum(const Parser::Field &field, E &val) {
        Validate_(field, 1, Parser::ValueType::kString);
        return Util::EnumFromString<E>(field.GetValue<std::string>(), val);
    }

  private:
    static void Validate_(const Parser::Field &field,
                          uint32_t count, Parser::ValueType type);
};

}  // namespace Input
