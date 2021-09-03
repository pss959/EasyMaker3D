#pragma once

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

    //! Derived classes must implement this to write a value using the given
    //! ValueWriter.
    virtual void WriteValue(ValueWriter &writer) const = 0;

  protected:
    //! The constructor is protected to make this abstract. It is passed the
    //! name of the field.
    Field(const std::string &name) : name_(name) {}

    //! Sets the flag indicating the field value was parsed or set.
    void SetWasParsed(bool was_parsed) { was_parsed_ = was_parsed; }

  private:
    std::string name_;                //!< Name of the field.
    bool        was_parsed_ = false;  //!< Whether the field was parsed.

    friend class Parser;
};

//! Abstract base class for a field that stores a value of the templated type.
template <typename T>
class TypedField : public Field {
  public:
    //! Constructor that is passed just the name of the field. The value will
    //! have the default value for its type.
    TypedField(const std::string &name) : Field(name) {}

    //! Constructor that is passed the name of the field and a default value.
    TypedField(const std::string &name, const T &def_val) :
        Field(name), value_(def_val) {}

    //! Explicit access to the wrapped value.
    T & GetValue()             { return value_; }
    //! Explicit access to the wrapped value.
    const T & GetValue() const { return value_; }

    //! Implicit cast to the wrapped value.
    operator T&()             { return value_; }
    //! Implicit cast to the wrapped value.
    operator const T&() const { return value_; }

    //! Allows a value to be set.
    void Set(const T &new_value) {
        value_ = new_value;
        SetWasParsed(true);
    }

    //! Assignment operator.
    TypedField<T> & operator=(const T &new_value) {
        Set(new_value);
        return *this;
    }

  protected:
    T value_;  //!< Value storage.
};

//! Derived field that stores a value of the templated type.
template <typename T> class TField : public TypedField<T> {
  public:
    //! Constructor that is passed just the name of the field. The value will
    //! have the default value for its type.
    TField(const std::string &name) : TypedField<T>(name) {}

    //! Constructor that is passed the name of the field and a default value.
    TField(const std::string &name, const T &def_val) :
        TypedField<T>(name, def_val) {}

    virtual void ParseValue(Scanner &scanner) override;

    virtual void WriteValue(ValueWriter &writer) const override {
        writer.WriteValue<T>(TypedField<T>::value_);
    }

    //! Assignment operator.
    TField<T> & operator=(const T &new_value) {
        TypedField<T>::Set(new_value);
        return *this;
    }
};

//! Derived field that stores an enum of some type.
template <typename E> class EnumField : public TypedField<E> {
  public:
    //! Constructor that is passed just the name of the field. The value will
    //! have the default value for its type.
    EnumField(const std::string &name) : TypedField<E>(name) {}

    //! Constructor that is passed the name of the field and a default value.
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

    virtual void WriteValue(ValueWriter &writer) const override {
        writer.WriteFlags<E>(TypedField<FlagType>::value_);
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

    virtual void WriteValue(ValueWriter &writer) const override {
        // This should not be called unless a real object was parsed.
        ASSERT(TypedField<PtrType>::value_);
        writer.WriteObject(*TypedField<PtrType>::value_);
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

    virtual void WriteValue(ValueWriter &writer) const override {
        std::vector<ObjectPtr> obj_list;
        for (const auto &obj: TypedField<ListType>::value_)
            obj_list.push_back(obj);
        writer.WriteObjectList(obj_list);
    }
};

}  // namespace Parser
