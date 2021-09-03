#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Parser/ObjectList.h"
#include "Parser/Scanner.h"
#include "Util/Enum.h"
#include "Util/Flags.h"
#include "Util/General.h"

namespace Parser {

class Scanner;

//! A field represents a single item inside an Object. It has a name and a flag
//! indicating whether the value was parsed. The FieldBase base class is
//! abstract; there are derived classes that wrap actual values.
class Field {
  public:
    virtual ~Field() {}

    //! Returns the name for the field.
    const std::string & GetName() const { return name_; }

    //! Returns true if the field was parsed.
    bool WasParsed() const { return was_parsed_; }

    //! Parses a value for the field using the given Scanner. Throws an
    //! exception using the Scanner if anything goes wrong. Derived classes
    //! must implement this.
    virtual void ParseValue(Scanner &scanner) = 0;

  protected:
    //! The constructor is protected to make this abstract. It is passed the
    //! name of the field.
    Field(const std::string &name) : name_(name) {}

  private:
    std::string name_;                //!< Name of the field.
    bool        was_parsed_ = false;  //!< Whether the field was parsed.

    friend class Parser;
};

//! Abstract base class for a field that stores a value of the templated type.
template <typename T>
class TypedField : public Field {
  public:
    TypedField(const std::string &name) : Field(name) {}

    //! Explicit access to the wrapped value.
    T & GetValue()             { return value_; }
    //! Explicit access to the wrapped value.
    const T & GetValue() const { return value_; }

    //! Implicit cast to the wrapped value.
    operator T&()             { return value_; }
    //! Implicit cast to the wrapped value.
    operator const T&() const { return value_; }

  protected:
    T value_;  //!< Value storage.
};

//! Derived field that stores an enum of some type.
template <typename E> class EnumField : public TypedField<E> {
  public:
    EnumField(const std::string &name) : TypedField<E>(name) {}
    virtual void ParseValue(Scanner &scanner) override {
        const std::string &str = scanner.ScanQuotedString();
        if (! Util::EnumFromString<E>(str, TypedField<E>::value_))
            scanner.Throw("Invalid value for enum: '" + str + "'");
    }
};

//! Derived field that stores a value of the templated type.
template <typename T> class TField : public TypedField<T> {
  public:
    TField(const std::string &name) : TypedField<T>(name) {}
    virtual void ParseValue(Scanner &scanner) override;
};

//! Derived field that stores a flag enum of some type.
template <typename E> class FlagField : public TypedField<Util::Flags<E>> {
  public:
    typedef Util::Flags<E> FlagType;

    FlagField(const std::string &name) : TypedField<FlagType>(name) {}
    virtual void ParseValue(Scanner &scanner) override {
        const std::string &str = scanner.ScanQuotedString();
        if (! FlagType::FromString(str, TypedField<FlagType>::value_))
            scanner.Throw("Invalid value for flag enum: '" + str + "'");
    }
};

//! Derived field that stores a shared_ptr to an object of some type.
template <typename T>
class ObjectField : public TypedField<std::shared_ptr<T>> {
  public:
    typedef std::shared_ptr<T> PtrType;

    ObjectField(const std::string &name) : TypedField<PtrType>(name) {}
    virtual void ParseValue(Scanner &scanner) override {
        ObjectPtr obj = scanner.ScanObject();
        TypedField<PtrType>::value_ = Util::CastToDerived<Object, T>(obj);
    }
};

//! Derived field that stores a vector of shared_ptrs to an object of some
//! type.
template <typename T>
class ObjectListField : public TypedField<std::vector<std::shared_ptr<T>>> {
  public:
    typedef std::vector<std::shared_ptr<T>> ListType;

    ObjectListField(const std::string &name) : TypedField<ListType>(name) {}
    virtual void ParseValue(Scanner &scanner) override {
        ObjectListPtr list = scanner.ScanObjectList();
        for (auto &obj: list->objects)
            TypedField<ListType>::value_.push_back(
                Util::CastToDerived<Object, T>(obj));
    }
};

}  // namespace Parser
