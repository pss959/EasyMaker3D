#pragma once

#include <string>
#include <vector>

#include "NParser/SpecBuilder.h"
#include "SG/Conversion.h"
#include "SG/Math.h"

namespace SG {

//! The SG::SpecBuilder class is a derived NParser::SpecBuilder, adding some
//! conveniences and SG-specific types for building functions.
template <typename OBJ> class SpecBuilder : public NParser::SpecBuilder<OBJ> {
  public:
    SpecBuilder() {}
    SpecBuilder(const std::vector<NParser::FieldSpec> &base_specs) :
        Base_(base_specs) {}

    //! Adds a FieldSpec for a bool field.
    void AddBool(const std::string &name, bool OBJ::* loc) {
        Base_::AddSingle(name, NParser::ValueType::kBool, loc);
    }

    //! Adds a FieldSpec for an int field.
    void AddInt(const std::string &name, int OBJ::* loc) {
        Base_::AddSingle(name, NParser::ValueType::kInteger, loc);
    }

    //! Adds a FieldSpec for a float field.
    void AddFloat(const std::string &name, float OBJ::* loc) {
        Base_::AddSingle(name, NParser::ValueType::kFloat, loc);
    }

    //! Adds a FieldSpec for a string field.
    void AddString(const std::string &name, std::string OBJ::* loc) {
        Base_::AddSingle(name, NParser::ValueType::kString, loc);
    }

    //! Adds a FieldSpec for a (string) field containing an enum of the
    //! templated type..
    template <typename E>
    void AddEnum(const std::string &name, E OBJ::* loc) {
        Base_::Add(
            NParser::FieldSpec(
                name, NParser::ValueType::kFloat, 1,
                [loc](NParser::Object &obj,
                      const std::vector<NParser::Value> &vals){
                    static_cast<OBJ&>(obj).*loc =
                        Conversion::ToEnum<E>(vals);}));
    }

    //! Adds a FieldSpec for an Anglef field.
    void AddAnglef(const std::string &name, Anglef OBJ::* loc) {
        Base_::Add(
            NParser::FieldSpec(
                name, NParser::ValueType::kFloat, 1,
                [loc](NParser::Object &obj,
                      const std::vector<NParser::Value> &vals){
                    static_cast<OBJ&>(obj).*loc =
                        Conversion::ToAnglef(vals);}));
    }

    //! Adds a FieldSpec for a Vector3f field.
    void AddVector3f(const std::string &name, Vector3f OBJ::* loc) {
        Base_::Add(
            NParser::FieldSpec(
                name, NParser::ValueType::kFloat, 3,
                [loc](NParser::Object &obj,
                      const std::vector<NParser::Value> &vals){
                    static_cast<OBJ&>(obj).*loc =
                        Conversion::ToVector3f(vals);}));
    }

    //! Adds a FieldSpec for a Vector4f field.
    void AddVector4f(const std::string &name, Vector4f OBJ::* loc) {
        Base_::Add(
            NParser::FieldSpec(
                name, NParser::ValueType::kFloat, 4,
                [loc](NParser::Object &obj,
                      const std::vector<NParser::Value> &vals){
                    static_cast<OBJ&>(obj).*loc =
                        Conversion::ToVector4f(vals);}));
    }

    //! Adds a FieldSpec for a Rotationf field.
    void AddRotationf(const std::string &name, Rotationf OBJ::* loc) {
        Base_::Add(
            NParser::FieldSpec(
                name, NParser::ValueType::kFloat, 4,
                [loc](NParser::Object &obj,
                      const std::vector<NParser::Value> &vals){
                    static_cast<OBJ&>(obj).*loc =
                        Conversion::ToRotationf(vals);}));
    }

  private:
    typedef NParser::SpecBuilder<OBJ> Base_;
};

}  // namespace NParser
