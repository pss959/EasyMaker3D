#pragma once

#include <memory>
#include <string>
#include <vector>

#include "NParser/FieldSpecs.h"
#include "NParser/ValueType.h"
#include "NParser/Value.h"
#include "Util/General.h"

namespace NParser {

//! Object is an abstract base class for all objects resulting from parsing.
//!
//! \ingroup NParser
class Object {
  public:
    virtual ~Object() {}

    //! Returns the type name for the object.
    const std::string & GetTypeName() const { return type_name_; }

    //! Returns the name assigned to the object, which may be empty.
    const std::string & GetName() const { return name_; }

    //! XXXX
    virtual const FieldSpecs & GetFieldSpecs() const = 0;

  protected:
    //! The constructor is protected to make this abstract.
    Object() {}

    //! Convenient Spec builder.
    static FieldSpecs::Spec BuildSpec(const std::string &name, ValueType type,
                                      int count,
                                      const FieldSpecs::StoreFunc &func) {
        FieldSpecs::Spec spec;
        spec.name       = name;
        spec.type       = type;
        spec.count      = count;
        spec.store_func = func;
        return spec;
    }

    // XXXX
    template <typename OBJ, typename VAL>
    static FieldSpecs::Spec SingleSpec(const std::string &name, ValueType type,
                                       VAL OBJ::* loc) {
        return BuildSpec(
            name, type, 1,
            [loc](Object &obj, const std::vector<NParser::Value> &vals){
                static_cast<OBJ&>(obj).*loc = std::get<VAL>(vals[0]);});
    }

    // XXXX
    template <typename OBJ, typename VAL, int N>
    static FieldSpecs::Spec ArraySpec(const std::string &name, ValueType type,
                                      VAL (OBJ::* loc)[N]) {
        return BuildSpec(
            name, type, N,
            [loc](Object &obj, const std::vector<NParser::Value> &vals){
                std::transform(vals.begin(), vals.end(),
                               static_cast<OBJ&>(obj).*loc,
                               [](const auto &v){ return std::get<VAL>(v); });});
    }

    // XXXX
    template <typename OBJ, typename FIELD_OBJ>
    static FieldSpecs::Spec ObjectSpec(const std::string &name,
                                       std::shared_ptr<FIELD_OBJ> OBJ::* loc) {
        return BuildSpec(
            name, ValueType::kObject, 1,
            [loc](Object &obj, const std::vector<NParser::Value> &vals){
                static_cast<OBJ&>(obj).*loc =
                    Util::CastToDerived<Object, FIELD_OBJ>(
                        std::get<std::shared_ptr<Object>>(vals[0]));});
    }

  private:
    std::string type_name_;  //!< Name of the object's type.
    std::string name_;       //!< Optional name assigned in file.

    //! Sets the type name for the object.
    void SetTypeName_(const std::string &type_name) { type_name_ = type_name; }

    //! Sets the name in an instance.
    void SetName_(const std::string &name) { name_ = name; }

    friend class Parser;
};

// XXXX
typedef std::shared_ptr<Object> ObjectPtr;

}  // namespace Parser
