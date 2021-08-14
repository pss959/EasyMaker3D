#include "Graph/Conversion.h"

#include <assert.h>

using ion::math::Anglef;
using ion::math::Matrix2f;
using ion::math::Matrix3f;
using ion::math::Matrix4f;
using ion::math::Rotationf;
using ion::math::Vector2f;
using ion::math::Vector3f;
using ion::math::Vector4f;
using ion::math::Vector2i;
using ion::math::Vector3i;
using ion::math::Vector4i;
using ion::math::Vector2ui;
using ion::math::Vector3ui;
using ion::math::Vector4ui;

namespace Graph {

Anglef Conversion::ToAnglef(const Parser::Field &field) {
    Validate_(field, 1, Parser::ValueType::kFloat);
    return Anglef::FromDegrees(field.GetValue<float>());
}

Vector2f Conversion::ToVector2f(const Parser::Field &field) {
    Validate_(field, 2, Parser::ValueType::kFloat);
    const std::vector<float> values = field.GetValues<float>();
    return Vector2f(values[0], values[1]);
}

Vector3f Conversion::ToVector3f(const Parser::Field &field) {
    Validate_(field, 3, Parser::ValueType::kFloat);
    const std::vector<float> values = field.GetValues<float>();
    return Vector3f(values[0], values[1], values[2]);
}

Vector4f Conversion::ToVector4f(const Parser::Field &field) {
    Validate_(field, 4, Parser::ValueType::kFloat);
    const std::vector<float> values = field.GetValues<float>();
    return Vector4f(values[0], values[1], values[2], values[3]);
}

Vector2i Conversion::ToVector2i(const Parser::Field &field) {
    Validate_(field, 2, Parser::ValueType::kInteger);
    const std::vector<int> values = field.GetValues<int>();
    return Vector2i(values[0], values[1]);
}

Vector3i Conversion::ToVector3i(const Parser::Field &field) {
    Validate_(field, 3, Parser::ValueType::kInteger);
    const std::vector<int> values = field.GetValues<int>();
    return Vector3i(values[0], values[1], values[2]);
}

Vector4i Conversion::ToVector4i(const Parser::Field &field) {
    Validate_(field, 4, Parser::ValueType::kInteger);
    const std::vector<int> values = field.GetValues<int>();
    return Vector4i(values[0], values[1], values[2], values[3]);
}

Vector2i Conversion::ToVector2ui(const Parser::Field &field) {
    Validate_(field, 2, Parser::ValueType::kUInteger);
    const std::vector<int> values = field.GetValues<int>();
    return Vector2i(values[0], values[1]);
}

Vector3i Conversion::ToVector3ui(const Parser::Field &field) {
    Validate_(field, 3, Parser::ValueType::kUInteger);
    const std::vector<int> values = field.GetValues<int>();
    return Vector3i(values[0], values[1], values[2]);
}

Vector4i Conversion::ToVector4ui(const Parser::Field &field) {
    Validate_(field, 4, Parser::ValueType::kUInteger);
    const std::vector<int> values = field.GetValues<int>();
    return Vector4i(values[0], values[1], values[2], values[3]);
}

Rotationf Conversion::ToRotationf(const Parser::Field &field) {
    const Vector4f vec = ToVector4f(field);
    return Rotationf::FromAxisAndAngle(Vector3f(vec[0], vec[1], vec[2]),
                                       Anglef::FromDegrees(vec[3]));
}

Matrix2f Conversion::ToMatrix2f(const Parser::Field &field) {
    Validate_(field, 4, Parser::ValueType::kFloat);
    const std::vector<float> values = field.GetValues<float>();
    return Matrix2f(values[0], values[1],
                    values[2], values[3]);
}

Matrix3f Conversion::ToMatrix3f(const Parser::Field &field) {
    Validate_(field, 9, Parser::ValueType::kFloat);
    const std::vector<float> values = field.GetValues<float>();
    return Matrix3f(values[0], values[1], values[2],
                    values[3], values[4], values[5],
                    values[6], values[7], values[8]);
}

Matrix4f Conversion::ToMatrix4f(const Parser::Field &field) {
    Validate_(field, 16, Parser::ValueType::kFloat);
    const std::vector<float> values = field.GetValues<float>();
    return Matrix4f(values[0], values[1], values[2], values[3],
                    values[4], values[5], values[6], values[7],
                    values[8], values[9], values[10], values[11],
                    values[12], values[13], values[14], values[15]);
}

void Conversion::Validate_(const Parser::Field &field,
                           uint32_t count, Parser::ValueType type) {
    assert(field.spec.type  == type);
    assert(field.spec.count == count);
}

}  // namespace Graph
