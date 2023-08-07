#include <concepts>
#include <memory>

#include "Parser/Object.h"
#include "Parser/Registry.h"
#include "Tests/TestBase.h"

// Derived TestBase class that automatically registers and unregisters known
// types on construction and destruction.
class TestBaseWithTypes : public TestBase {
  protected:
    TestBaseWithTypes();

    virtual ~TestBaseWithTypes();

    // Convenience to create an Object of the (known) templated type derived
    // from Parser::Object.
    template <typename T>
    static std::shared_ptr<T> CreateObject(const std::string &name = "") {
        static_assert(std::derived_from<T, Parser::Object> == true);
        return Parser::Registry::CreateObject<T>(name);
    }

    // Tries to parse an object of the templated type (derived from
    // Parser::Object) from the given string. Returns a null pointer on
    // failure.
    template <typename T>
    std::shared_ptr<T> ParseObject(const std::string &input) {
        static_assert(std::derived_from<T, Parser::Object> == true);
        return std::dynamic_pointer_cast<T>(ParseItem_(input));
    }

  private:
    static Parser::ObjectPtr ParseItem_(const std::string &input);
};
