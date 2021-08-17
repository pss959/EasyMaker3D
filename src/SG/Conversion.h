#pragma once

#include <vector>

#include "NParser/Value.h"
#include "NParser/ValueType.h"
#include "SG/Math.h"

//! This class contains convenient static functions to convert NParser::Value
//! types to SG/Ion types. Each asserts if used incorrectly.
namespace SG {

class Conversion {
  public:
    static Anglef    ToAnglef(const    std::vector<NParser::Value> &vals);
    static Vector2f  ToVector2f(const  std::vector<NParser::Value> &vals);
    static Vector3f  ToVector3f(const  std::vector<NParser::Value> &vals);
    static Vector4f  ToVector4f(const  std::vector<NParser::Value> &vals);
    static Vector2i  ToVector2i(const  std::vector<NParser::Value> &vals);
    static Vector3i  ToVector3i(const  std::vector<NParser::Value> &vals);
    static Vector4i  ToVector4i(const  std::vector<NParser::Value> &vals);
    static Vector2ui ToVector2ui(const std::vector<NParser::Value> &vals);
    static Vector3ui ToVector3ui(const std::vector<NParser::Value> &vals);
    static Vector4ui ToVector4ui(const std::vector<NParser::Value> &vals);
    static Rotationf ToRotationf(const std::vector<NParser::Value> &vals);
    static Matrix2f  ToMatrix2f(const  std::vector<NParser::Value> &vals);
    static Matrix3f  ToMatrix3f(const  std::vector<NParser::Value> &vals);
    static Matrix4f  ToMatrix4f(const  std::vector<NParser::Value> &vals);
};

}  // namespace SG
