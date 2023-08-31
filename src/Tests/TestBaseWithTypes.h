#pragma once

#include <concepts>
#include <memory>

#include "Parser/Object.h"
#include "Parser/Registry.h"
#include "Tests/TestBase.h"

/// Derived TestBase class that automatically registers and unregisters known
/// types on construction and destruction and provides some conveniences for
/// creating instances of known types.
///
/// \ingroup Tests
class TestBaseWithTypes : public TestBase {
  protected:
    TestBaseWithTypes();

    virtual ~TestBaseWithTypes();

    /// Convenience to create an Object of the (known) templated type derived
    /// from Parser::Object.
    template <typename T>
    static std::shared_ptr<T> CreateObject(const Str &name = "") {
        static_assert(std::derived_from<T, Parser::Object> == true);
        return Parser::Registry::CreateObject<T>(name);
    }

    /// Tries to parse an object of some type (derived from Parser::Object)
    /// from the given string. Returns a null pointer on failure.
    static Parser::ObjectPtr ParseObject(const Str &input);

    /// Tries to parse an object of the templated type (derived from
    /// Parser::Object) from the given string. Returns a null pointer on
    /// failure.
    template <typename T>
    static std::shared_ptr<T> ParseTypedObject(const Str &input) {
        static_assert(std::derived_from<T, Parser::Object> == true);
        return std::dynamic_pointer_cast<T>(ParseObject(input));
    }

    /// Sets the type name to use for parsing in TestValid() and TestInvalid().
    void SetParseTypeName(const Str &type_name) {
        parse_type_name_ = type_name;
    }

    /// Returns the name passed to SetParseTypeName().
    const Str & GetParseTypeName() const { return parse_type_name_; }

    /// This can be used for testing the IsValid() function for classes derived
    /// from Parser::Object. It verifies that parsing an object of the type set
    /// by SetParseTypeName() with the given contents returns a valid
    /// (non-null) instance. This asserts if SetParseTypeName() was not called.
    void TestValid(const Str &contents);

    /// This can be used for testing the IsValid() function for classes derived
    /// from Parser::Object. It verifies that a Parser::Exception is thrown
    /// containing the given error message when parsing an object of the type set
    /// by SetParseTypeName() with the given contents.
    void TestInvalid(const Str &contents, const Str &error);

    /// Convenience that builds a string using the name passed to
    /// GetParseTypeName() and the given contents. This asserts if
    /// SetParseTypeName() was not called.
    Str BuildParseString(const Str &contents) const;

  private:
    Str parse_type_name_;
};

