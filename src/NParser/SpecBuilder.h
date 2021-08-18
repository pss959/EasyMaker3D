#pragma once

#include <string>
#include <vector>

#include "NParser/FieldSpec.h"
#include "NParser/Object.h"
#include "NParser/Value.h"
#include "NParser/ValueType.h"
#include "Util/General.h"

namespace NParser {

//! The SpecBuilder class helps build a FieldSpecs instance. It is templated by
//! the derived NParser::Object class that owns the specs.
template <typename OBJ> class SpecBuilder {
  public:
    SpecBuilder() {}
    SpecBuilder(const std::vector<FieldSpec> &base_specs) {
        specs_ = base_specs;
    }

    //! Returns the resulting specs.
    std::vector<FieldSpec> GetSpecs() { return specs_; }

    //! Adds a pre-built FieldSpec instance.
    void Add(const FieldSpec &spec) { specs_.push_back(spec); }

    //! Adds a field spec for a single-valued field of the given type. The
    //! location to store the parsed value is supplied.
    template <typename VAL>
    void AddSingle(const std::string &name, ValueType type, VAL OBJ::* loc) {
        specs_.push_back(
            FieldSpec(
                name, type, 1,
                [loc](Object &obj, const std::vector<NParser::Value> &vals){
                    static_cast<OBJ&>(obj).*loc = std::get<VAL>(vals[0]);}));
    }

    //! Adds a field spec for a multi-valued field of the given type with the
    //! given number of values. The location to store the parsed values
    //! (contiguously) is supplied.
    template <typename VAL, int N>
    void AddArray(const std::string &name, ValueType type,
                  VAL (OBJ::* loc)[N]) {
        specs_.push_back(
            FieldSpec(
                name, type, N,
                [loc](Object &obj, const std::vector<NParser::Value> &vals){
                    std::transform(vals.begin(), vals.end(),
                                   static_cast<OBJ&>(obj).*loc,
                                   [](const auto &v){
                                       return std::get<VAL>(v); });}));
    }

    //! Adds a field spec for a single-valued field containing an object of the
    //! type VOBJ. The location of the shared_ptr to store the object in
    //! is supplied.
    template <typename VOBJ>
    void AddObject(const std::string &name, std::shared_ptr<VOBJ> OBJ::* loc) {
        specs_.push_back(
            FieldSpec(
                name, ValueType::kObject, 1,
                [loc](Object &obj, const std::vector<NParser::Value> &vals){
                    static_cast<OBJ&>(obj).*loc =
                        Util::CastToDerived<Object, VOBJ>(
                            std::get<ObjectPtr>(vals[0]));}));
    }

    //! Adds a field spec for a single-valued field containing a vector of
    //! objects of the type VOBJ. The location of the vector of shared_ptr
    //! instances to store the objects in is supplied.
    template <typename VOBJ>
    void AddObjectList(const std::string &name,
                       std::vector<std::shared_ptr<VOBJ>> OBJ::* loc) {
        specs_.push_back(
            FieldSpec(
                name, ValueType::kObjectList, 1,
                [loc](Object &obj, const std::vector<NParser::Value> &vals){
                    auto &in_vec  = std::get<std::vector<ObjectPtr>>(vals[0]);
                    auto &out_vec = static_cast<OBJ&>(obj).*loc;
                    std::transform(in_vec.begin(), in_vec.end(),
                                   std::back_inserter(out_vec),
                                   [](const auto &v){
                                       return Util::CastToDerived<Object,
                                                                  VOBJ>(v);});
                }));
    }

  private:
    std::vector<FieldSpec> specs_;
};

}  // namespace NParser
