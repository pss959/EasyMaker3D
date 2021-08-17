#pragma once

#include <string>
#include <vector>

#include "NParser/SpecBuilder.h"
#include "SG/Conversion.h"
#include "SG/Math.h"

namespace SG {

//! The SG::SpecBuilder class is a derived NParser::SpecBuilder, adding some
//! SG-specific types for building functions.
//!
//! \ingroup SG
template <typename OBJ> class SpecBuilder : public NParser::SpecBuilder<OBJ> {
  public:
    SpecBuilder() {}
    SpecBuilder(const std::vector<NParser::FieldSpec> &base_specs) :
        Base_(base_specs) {}

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
