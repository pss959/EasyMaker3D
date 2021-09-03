#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Parser/Scanner.h"
#include "Util/Enum.h"
#include "Util/Flags.h"

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

//! Derived field that stores a value of the templated type.
template <typename T>
class TField : public Field {
  public:
    TField(const std::string &name) : Field(name) {}
    virtual void ParseValue(Scanner &scanner) override;

    //! Explicit access to the wrapped value.
    T & GetValue()             { return value_; }
    //! Explicit access to the wrapped value.
    const T & GetValue() const { return value_; }

    //! Implicit cast to the wrapped value.
    operator T&()             { return value_; }
    //! Implicit cast to the wrapped value.
    operator const T&() const { return value_; }

  private:
    T value_;  //!< Value storage.
};

//! Derived field that stores an enum of some type.
template <typename E>
class EnumField : public Field {
  public:
    EnumField(const std::string &name) : Field(name) {}
    virtual void ParseValue(Scanner &scanner) override {
        const std::string &str = scanner.ScanQuotedString();
        if (! Util::EnumFromString<E>(str, value_))
            scanner.Throw("Invalid value for enum: '" + str + "'");
    }

    //! Explicit access to the wrapped value.
    E & GetValue()             { return value_; }
    //! Explicit access to the wrapped value.
    const E & GetValue() const { return value_; }

    //! Implicit cast to the wrapped value.
    operator E &()             { return value_; }
    //! Implicit cast to the wrapped value.
    operator const E &() const { return value_; }

  private:
    E value_;
};

//! Derived field that stores a flag enum of some type.
template <typename E>
class FlagField : public Field {
  public:
    typedef Util::Flags<E> FlagType;

    FlagField(const std::string &name) : Field(name) {}
    virtual void ParseValue(Scanner &scanner) override {
        const std::string &str = scanner.ScanQuotedString();
        if (! FlagType::FromString(str, value_))
            scanner.Throw("Invalid value for flag enum: '" + str + "'");
    }

    //! Explicit access to the wrapped value.
    FlagType & GetValue()             { return value_; }
    //! Explicit access to the wrapped value.
    const FlagType & GetValue() const { return value_; }

    //! Implicit cast to the wrapped value.
    operator FlagType &()             { return value_; }
    //! Implicit cast to the wrapped value.
    operator const FlagType &() const { return value_; }

  private:
    FlagType value_;
};

//! Derived field that stores a shared_ptr to an object of some type.
template <typename T>
class ObjectField : public Field {
  public:
    ObjectField(const std::string &name) : Field(name) {}
    virtual void ParseValue(Scanner &scanner) override;

  private:
    std::shared_ptr<T> object_;  //!< Value storage.
};

//! Derived field that stores a vector of shared_ptrs to an object of some
//! type.
template <typename T>
class ObjectListField : public Field {
  public:
    ObjectListField(const std::string &name) : Field(name) {}
    virtual void ParseValue(Scanner &scanner) override;

  private:
    std::vector<std::shared_ptr<T>> objects_;  //!< Value storage.
};

typedef std::shared_ptr<Field> FieldPtr;

}  // namespace Parser
