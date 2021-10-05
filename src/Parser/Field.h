#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <Assert.h>
#include "Parser/ObjectList.h"
#include "Parser/Scanner.h"
#include "Parser/ValueWriter.h"
#include "Util/Enum.h"
#include "Util/Flags.h"
#include "Util/General.h"

namespace Parser {

class Scanner;

/// A field represents a single item inside an Object. It has a name and a flag
/// indicating whether the value was parsed or otherwise set. The base Field
/// class is abstract; there are derived classes that wrap actual values.
class Field {
  public:
    virtual ~Field() {}

    /// Returns the name for the field.
    const std::string & GetName() const { return name_; }

    /// Returns true if the field was parsed or otherwise set.
    bool WasSet() const { return was_set_; }

    /// Parses a value for the field using the given Scanner. Throws an
    /// exception using the Scanner if anything goes wrong. Derived classes
    /// must implement this.
    virtual void ParseValue(Scanner &scanner) = 0;

    /// Derived classes must implement this to write a value using the given
    /// ValueWriter.
    virtual void WriteValue(ValueWriter &writer) const = 0;

    /// Copies the value from another field, which must be the same type. If
    /// is_deep is true, fields containing Object pointers will clone their
    /// objects.
    virtual void CopyFrom(const Field &from, bool is_deep) = 0;

  protected:
    /// The constructor is protected to make this abstract. It is passed the
    /// name of the field.
    Field(const std::string &name) : name_(name) {}

    /// Sets the flag indicating the field value was parsed or set.
    void SetWasSet(bool was_set) { was_set_ = was_set; }

    void ThrowObjectTypeError(Scanner &scanner, const ObjectPtr &obj);

    /// Scans a value of the templated type. This is instantiated in the source
    /// file for all supported types. It is used by both TField and VField.
    template <typename T> T ScanValue(Scanner &scanner);

    /// Scans multiple values of some type. The function is invoked when it is
    /// time to scan a value.
    void ScanValues(Scanner &scanner, const std::function<void()> &scan_func);

  private:
    std::string name_;             ///< Name of the field.
    bool        was_set_ = false;  ///< Whether the field was parsed or set.

    friend class Parser;
};

/// Abstract base class for a field that stores a value of the templated type.
template <typename T>
class TypedField : public Field {
  public:
    /// Constructor that is passed just the name of the field. The value will
    /// have the default value for its type.
    TypedField(const std::string &name) : Field(name) {}

    /// Constructor that is passed the name of the field and a default value.
    TypedField(const std::string &name, const T &def_val) :
        Field(name), value_(def_val) {}

    /// Explicit access to the wrapped value.
    T & GetValue()             { return value_; }
    /// Explicit access to the wrapped value.
    const T & GetValue() const { return value_; }

    /// Implicit cast to the wrapped value.
    operator T&()             { return value_; }
    /// Implicit cast to the wrapped value.
    operator const T&() const { return value_; }

    /// Allows a value to be set.
    void Set(const T &new_value) {
        value_ = new_value;
        SetWasSet(true);
    }

    /// Assignment operator.
    TypedField<T> & operator=(const T &new_value) {
        Set(new_value);
        return *this;
    }

    virtual void CopyFrom(const Field &from, bool is_deep) override {
        ASSERT(from.GetName() == GetName());
        value_ = static_cast<const TypedField<T> &>(from).value_;
        SetWasSet(from.WasSet());
    }

  protected:
    T value_;  ///< Value storage.
};

/// Derived field that stores a value of the templated type.
template <typename T> class TField : public TypedField<T> {
  public:
    /// Constructor that is passed just the name of the field. The value will
    /// have the default value for its type.
    TField(const std::string &name) : TypedField<T>(name) {}

    /// Constructor that is passed the name of the field and a default value.
    TField(const std::string &name, const T &def_val) :
        TypedField<T>(name, def_val) {}

    virtual void ParseValue(Scanner &scanner) override {
        TypedField<T>::value_ = Field::ScanValue<T>(scanner);
    }

    virtual void WriteValue(ValueWriter &writer) const override {
        writer.WriteValue<T>(TypedField<T>::value_);
    }

    /// Assignment operator.
    TField<T> & operator=(const T &new_value) {
        TypedField<T>::Set(new_value);
        return *this;
    }
};

/// Derived field that stores a vector of values of the templated type.
template <typename T> class VField : public TypedField<std::vector<T>> {
  public:
    typedef std::vector<T> VecType;

    VField(const std::string &name) : TypedField<VecType>(name) {}

    virtual void ParseValue(Scanner &scanner) override {
        Field::ScanValues(
            scanner,
            [this, &scanner](){
            TypedField<VecType>::value_.push_back(Field::ScanValue<T>(scanner));
            });
    }

    virtual void WriteValue(ValueWriter &writer) const override {
        writer.WriteValueVector<T>(TypedField<VecType>::value_);
    }

    /// Assignment operator.
    VField<T> & operator=(const VecType &new_value) {
        TypedField<VecType>::Set(new_value);
        return *this;
    }
};

/// Derived field that stores an enum of some type.
template <typename E> class EnumField : public TypedField<E> {
  public:
    /// Constructor that is passed just the name of the field. The value will
    /// have the default value for its type.
    EnumField(const std::string &name) : TypedField<E>(name) {}

    /// Constructor that is passed the name of the field and a default value.
    EnumField(const std::string &name, E def_val) :
        TypedField<E>(name, def_val) {}

    virtual void ParseValue(Scanner &scanner) override {
        const std::string &str = scanner.ScanQuotedString();
        if (! Util::EnumFromString<E>(str, TypedField<E>::value_))
            scanner.Throw("Invalid value for enum: '" + str + "'");
    }

    virtual void WriteValue(ValueWriter &writer) const override {
        writer.WriteEnum<E>(TypedField<E>::value_);
    }

    /// Convenience function to get the name of the enum stored in the field.
    std::string GetEnumName() const {
        return Util::EnumName(TypedField<E>::value_);
    }

    /// Assignment operator.
    EnumField<E> & operator=(E new_value) {
        TypedField<E>::Set(new_value);
        return *this;
    }
};

/// Derived field that stores a flag enum of some type.
template <typename E> class FlagField : public TypedField<Util::Flags<E>> {
  public:
    typedef Util::Flags<E> FlagType;

    FlagField(const std::string &name) : TypedField<FlagType>(name) {}

    virtual void ParseValue(Scanner &scanner) override {
        const std::string &str = scanner.ScanQuotedString();
        if (! FlagType::FromString(str, TypedField<FlagType>::value_))
            scanner.Throw("Invalid value for flag enum: '" + str + "'");
    }

    /// Convenience to set a flag.
    void Set(E flag) {
        TypedField<FlagType>::value_.Set(flag);
        TypedField<FlagType>::SetWasSet(true);
    }

    /// Convenience to reset a flag.
    void Reset(E flag) {
        TypedField<FlagType>::value_.Reset(flag);
        TypedField<FlagType>::SetWasSet(true);
    }

    virtual void WriteValue(ValueWriter &writer) const override {
        writer.WriteFlags<E>(TypedField<FlagType>::value_);
    }
};

/// Derived field that stores a shared_ptr to an object of some type.
template <typename T>
class ObjectField : public TypedField<std::shared_ptr<T>> {
  public:
    typedef std::shared_ptr<T> PtrType;

    ObjectField(const std::string &name) : TypedField<PtrType>(name) {}
    virtual void ParseValue(Scanner &scanner) override {
        ObjectPtr obj = scanner.ScanObject();
        PtrType t = Util::CastToDerived<T>(obj);
        if (! t)
            Field::ThrowObjectTypeError(scanner, obj);
        TypedField<PtrType>::value_ = t;
    }

    virtual void WriteValue(ValueWriter &writer) const override {
        // This should not be called unless a real object was set.
        ASSERT(TypedField<PtrType>::value_);
        writer.WriteObject(*TypedField<PtrType>::value_);
    }

    /// Assignment operator.
    ObjectField<T> & operator=(const PtrType &ptr) {
        TypedField<PtrType>::value_ = ptr;
        TypedField<PtrType>::SetWasSet(true);
        return *this;
    }

    virtual void CopyFrom(const Field &from, bool is_deep) override {
        TypedField<PtrType>::CopyFrom(from, is_deep);
        if (is_deep) {
            auto &value = TypedField<PtrType>::value_;
            if (value)
                value = Util::CastToDerived<T>(value->Clone(true));
        }
    }
};

/// Derived field that stores a vector of shared_ptrs to an object of some
/// type.
template <typename T>
class ObjectListField : public TypedField<std::vector<std::shared_ptr<T>>> {
  public:
    typedef std::shared_ptr<T>   PtrType;
    typedef std::vector<PtrType> ListType;

    ObjectListField(const std::string &name) : TypedField<ListType>(name) {}

    virtual void ParseValue(Scanner &scanner) override {
        ObjectListPtr list = scanner.ScanObjectList();
        for (auto &obj: list->objects) {
            PtrType t = Util::CastToDerived<T>(obj);
            if (! t)
                Field::ThrowObjectTypeError(scanner, obj);
            TypedField<ListType>::value_.push_back(t);
        }
    }

    /// Adds an item to the vector.
    void Add(const PtrType &item) {
        ASSERT(item);
        TypedField<ListType>::value_.push_back(item);
        TypedField<ListType>::SetWasSet(true);
    }

    /// Inserts an item into the vector.
    void Insert(size_t index, const PtrType &item) {
        ASSERT(item);
        auto &vec = TypedField<ListType>::value_;
        ASSERT(index < vec.size());
        vec.insert(vec.begin() + index, item);
        TypedField<ListType>::SetWasSet(true);
    }

    /// Removes an item from the vector.
    void Remove(size_t index) {
        auto &vec = TypedField<ListType>::value_;
        ASSERT(index < vec.size());
        vec.erase(vec.begin() + index);
        TypedField<ListType>::SetWasSet(true);
    }

    /// Replaces an item in the vector.
    void Replace(size_t index, const PtrType &item) {
        auto &vec = TypedField<ListType>::value_;
        ASSERT(index < vec.size());
        vec.erase(vec.begin() + index);
        TypedField<ListType>::SetWasSet(true);
    }

    virtual void WriteValue(ValueWriter &writer) const override {
        std::vector<ObjectPtr> obj_list;
        for (const auto &obj: TypedField<ListType>::value_)
            obj_list.push_back(obj);
        writer.WriteObjectList(obj_list);
    }

    virtual void CopyFrom(const Field &from, bool is_deep) override {
        TypedField<ListType>::CopyFrom(from, is_deep);
        if (is_deep) {
            std::vector<PtrType> obj_list;
            for (const auto &obj: TypedField<ListType>::value_)
                obj_list.push_back(
                    obj ? Util::CastToDerived<T>(obj->Clone(is_deep)) : obj);
            TypedField<ListType>::value_ = obj_list;
        }
    }
};

}  // namespace Parser
