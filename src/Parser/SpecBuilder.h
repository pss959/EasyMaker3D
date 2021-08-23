#pragma once

#include <string>
#include <vector>

#include "Parser/Exception.h"
#include "Parser/FieldSpec.h"
#include "Parser/Object.h"
#include "Parser/Value.h"
#include "Parser/ValueType.h"
#include "Util/Enum.h"
#include "Util/Flags.h"
#include "Util/General.h"

namespace Parser {

//! The SpecBuilder class helps build a FieldSpecs instance. It is templated by
//! the derived Parser::Object class that owns the specs.
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
        const FieldSpec::StoreFunc func =
            [loc](Object &obj, const std::vector<Value> &vals){
                StoreSingle_<VAL>(obj, vals, loc); };
        specs_.push_back(FieldSpec(name, type, 1, func));
    }

    //! Adds a field spec for a multi-valued field of the given type with the
    //! given number of values. The location to store the parsed values
    //! (contiguously) is supplied.
    template <typename VAL, int N>
    void AddArray(const std::string &name, ValueType type,
                  VAL (OBJ::* loc)[N]) {
        const FieldSpec::StoreFunc func =
            [loc](Object &obj, const std::vector<Value> &vals){
                StoreArray_<VAL>(obj, vals, loc); };
        specs_.push_back(FieldSpec(name, type, N, func));
    }

    //! Adds a field spec for a single-valued field containing an object of the
    //! type VOBJ. The location of the shared_ptr to store the object in
    //! is supplied.
    template <typename VOBJ>
    void AddObject(const std::string &name, std::shared_ptr<VOBJ> OBJ::* loc) {
        const FieldSpec::StoreFunc func =
            [loc](Object &obj, const std::vector<Value> &vals){
                StoreObj_<VOBJ>(obj, vals, loc); };
        specs_.push_back(FieldSpec(name, ValueType::kObject, 1, func));
    }

    //! Adds a field spec for a single-valued field containing a vector of
    //! objects of the type VOBJ. The location of the vector of shared_ptr
    //! instances to store the objects in is supplied.
    template <typename VOBJ>
    void AddObjectList(const std::string &name,
                       std::vector<std::shared_ptr<VOBJ>> OBJ::* loc) {
        const FieldSpec::StoreFunc func =
            [loc](Object &obj, const std::vector<Value> &vals){
                StoreObjList_<VOBJ>(obj, vals, loc); };
        specs_.push_back(FieldSpec(name, ValueType::kObjectList, 1, func));
    }

    //! \name Typed convenience functions.
    //! Each of these adds a FieldSpec of a specific type.
    //!@{

    void AddBool(const std::string &name, bool OBJ::* loc) {
        AddSingle(name, ValueType::kBool, loc);
    }
    void AddInt(const std::string &name, int OBJ::* loc) {
        AddSingle(name, ValueType::kInteger, loc);
    }
    void AddUInt(const std::string &name, unsigned int OBJ::* loc) {
        AddSingle(name, ValueType::kUInteger, loc);
    }
    void AddFloat(const std::string &name, float OBJ::* loc) {
        AddSingle(name, ValueType::kFloat, loc);
    }
    void AddString(const std::string &name, std::string OBJ::* loc) {
        AddSingle(name, ValueType::kString, loc);
    }

    //! Adds a FieldSpec for a (string) field containing an enum of the
    //! templated type.
    template <typename E>
    void AddEnum(const std::string &name, E OBJ::* loc) {
        Add(FieldSpec(
                name, ValueType::kString, 1,
                [loc](Object &obj, const std::vector<Value> &vals){
                    static_cast<OBJ&>(obj).*loc = ConvertEnum_<E>(vals); }));
    }

    //! Adds a FieldSpec for a (string) field containing enum flags of the
    //! templated type.
    template <typename E>
    void AddFlags(const std::string &name, Util::Flags<E> OBJ::* loc) {
        Add(FieldSpec(
                name, ValueType::kString, 1,
                [loc](Object &obj, const std::vector<Value> &vals){
                    static_cast<OBJ&>(obj).*loc = ConvertFlags_<E>(vals); }));
    }

    //!@}

  private:
    std::vector<FieldSpec> specs_;

    //! Helper that stores a single value of type VAL in an object field.
    template <typename VAL>
    static void StoreSingle_(Object &obj, const std::vector<Value> &vals,
                             VAL OBJ::* loc) {
        static_cast<OBJ&>(obj).*loc = std::get<VAL>(vals[0]);
    }

    //! Helper that stores an array value of type VAL in an object field.
    template <typename VAL>
    static void StoreArray_(Object &obj, const std::vector<Value> &vals,
                            VAL (OBJ::* loc)[]) {
        std::transform(vals.begin(), vals.end(),
                       static_cast<OBJ&>(obj).*loc,
                       [](const auto &v){ return std::get<VAL>(v); });
    }

    //! Helper that stores an ObjectPtr value of the derived VOBJ type in an
    //! object field.
    template <typename VOBJ>
    static void StoreObj_(Object &obj, const std::vector<Value> &vals,
                          std::shared_ptr<VOBJ> OBJ::* loc) {
        static_cast<OBJ&>(obj).*loc =
            Util::CastToDerived<Object, VOBJ>(std::get<ObjectPtr>(vals[0]));
    }

    //! Helper that stores an array of ObjectPtr values of the derived VOBJ
    //! type in an object field.
    template <typename VOBJ>
    static void StoreObjList_(Object &obj, const std::vector<Value> &vals,
                              std::vector<std::shared_ptr<VOBJ>> OBJ::* loc) {
        auto &in_vec  = std::get<std::vector<ObjectPtr>>(vals[0]);
        auto &out_vec = static_cast<OBJ&>(obj).*loc;
        std::transform(in_vec.begin(), in_vec.end(),
                       std::back_inserter(out_vec),
                       [](const auto &v){
                           return Util::CastToDerived<Object, VOBJ>(v); });
    }

    //! Converts a Value storing a string representing an enum to the enum
    //! value.
    template <typename E>
    static E ConvertEnum_(const std::vector<Value> &vals) {
        E e;
        const std::string str = std::get<std::string>(vals[0]);
        if (! Util::EnumFromString<E>(str, e))
            throw Exception("Invalid value for enum: '" + str + "'");
        return e;
    }

    //! Converts a Value storing a string representing flags in a flag enum to
    //! the flags value.
    template <typename E>
    static Util::Flags<E> ConvertFlags_(const std::vector<Value> &vals) {
        Util::Flags<E> flags;
        const std::string &str = std::get<std::string>(vals[0]);
        if (! Util::Flags<E>::FromString(std::get<std::string>(vals[0]), flags))
            throw Exception("Invalid value for flag enum: '" + str + "'");
        return flags;
    }
};

}  // namespace Parser
