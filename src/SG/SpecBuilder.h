#pragma once

#include <string>
#include <vector>

#include "Parser/SpecBuilder.h"
#include "SG/Conversion.h"
#include "SG/Math.h"

namespace SG {

//! The SG::SpecBuilder class is a derived Parser::SpecBuilder, adding some
//! conveniences and SG-specific types for building functions.
template <typename OBJ> class SpecBuilder : public Parser::SpecBuilder<OBJ> {
  public:
    SpecBuilder() {}
    SpecBuilder(const std::vector<Parser::FieldSpec> &base_specs) :
        Base_(base_specs) {}

    //! Adds a FieldSpec for a field containing N values of the given value
    //! type, resulting in the templated type.
    template <typename RESULT, Parser::ValueType VAL, int N>
    void AddTyped(const std::string &name, RESULT OBJ::* loc) {
        Base_::Add(
            Parser::FieldSpec(
                name, VAL, N,
                [loc](Parser::Object &obj,
                      const std::vector<Parser::Value> &vals){
                    static_cast<OBJ&>(obj).*loc =
                        Conversion::To<RESULT>(vals);}));
    }

    //! Adds a FieldSpec for a float-based field containing N values, resulting
    //! in the templated type.
    template <typename RESULT, int N>
    void AddFloats(const std::string &name, RESULT OBJ::* loc) {
        AddTyped<RESULT, Parser::ValueType::kFloat, N>(name, loc);
    }

    //! Adds a FieldSpec for an int-based field containing N values, resulting
    //! in the templated type.
    template <typename RESULT, int N>
    void AddInts(const std::string &name, RESULT OBJ::* loc) {
        AddTyped<RESULT, Parser::ValueType::kInteger, N>(name, loc);
    }

    //! Adds a FieldSpec for an unsigned-int-based field containing N values,
    //! resulting in the templated type.
    template <typename RESULT, int N>
    void AddUInts(const std::string &name, RESULT OBJ::* loc) {
        AddTyped<RESULT, Parser::ValueType::kUInteger, N>(name, loc);
    }

    //! \name Typed convenience functions.
    //! Each of these adds a FieldSpec of a specific type.
    //!@{

    void AddAnglef(const std::string &name, Anglef OBJ::* loc) {
        AddFloats<Anglef, 1>(name, loc);
    }
    void AddVector2f(const std::string &name, Vector2f OBJ::* loc) {
        AddFloats<Vector2f, 2>(name, loc);
    }
    void AddVector3f(const std::string &name, Vector3f OBJ::* loc) {
        AddFloats<Vector3f, 3>(name, loc);
    }
    void AddVector4f(const std::string &name, Vector4f OBJ::* loc) {
        AddFloats<Vector4f, 4>(name, loc);
    }
    void AddVector2i(const std::string &name, Vector2i OBJ::* loc) {
        AddInts<Vector2i, 2>(name, loc);
    }
    void AddVector3i(const std::string &name, Vector3i OBJ::* loc) {
        AddInts<Vector3i, 3>(name, loc);
    }
    void AddVector4i(const std::string &name, Vector4i OBJ::* loc) {
        AddInts<Vector4i, 4>(name, loc);
    }
    void AddVector2ui(const std::string &name, Vector2ui OBJ::* loc) {
        AddUInts<Vector2ui, 2>(name, loc);
    }
    void AddVector3ui(const std::string &name, Vector3ui OBJ::* loc) {
        AddUInts<Vector3ui, 3>(name, loc);
    }
    void AddVector4ui(const std::string &name, Vector4ui OBJ::* loc) {
        AddUInts<Vector4ui, 4>(name, loc);
    }
    void AddRotationf(const std::string &name, Rotationf OBJ::* loc) {
        AddFloats<Rotationf, 4>(name, loc);
    }
    void AddMatrix2f(const std::string &name, Matrix2f OBJ::* loc) {
        AddFloats<Matrix2f, 2 * 2>(name, loc);
    }
    void AddMatrix3f(const std::string &name, Matrix3f OBJ::* loc) {
        AddFloats<Matrix3f, 3 * 3>(name, loc);
    }
    void AddMatrix4f(const std::string &name, Matrix4f OBJ::* loc) {
        AddFloats<Matrix4f, 4 * 4>(name, loc);
    }

    //!@}

  private:
    typedef Parser::SpecBuilder<OBJ> Base_;
};

}  // namespace Parser
