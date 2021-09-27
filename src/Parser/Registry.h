#pragma once

#include <functional>
#include <istream>
#include <functional>
#include <string>
#include <unordered_map>

#include "Assert.h"
#include "Parser/Object.h"
#include "Util/General.h"

namespace Parser {

/// The Parser::Registry class maintains a registry of concrete classes derived
/// from Parser::Object. Each entry in the registry is keyed by the name of the
/// class and contains a function that can be invoked to create an instance of
/// that class.
///
/// This is essentially a singleton since all functions and storage are static.
class Registry {
  public:
    /// Convenience typedef for the function used to create an object.
    typedef std::function<Object *()> CreationFunc;

    /// Registers a derived concrete Parser::Object class that can be parsed.
    /// The unique name to use for the class and a function that can be used to
    /// create an instance are supplied.
    template <typename T>
    static void AddType(const std::string &type_name) {
        // Make sure the class does not have a public default constructor.
        ASSERTM(! std::is_default_constructible<T>::value,
                "Class " + type_name + " has a public constructor");
        AddType_(type_name, []{ return new T; });
    }

    /// Calls the CreationFunc for the given object type, sets up the resulting
    /// instance, and returns it. Throws an Exception if the type was not
    /// registered.
    static ObjectPtr CreateObjectOfType(const std::string &type_name);

    /// Convenience that uses CreateObjectOfType() to create an object and then
    /// casts it to the target type. Throws an Exception if the type was not
    /// registered. Returns a null pointer if the cast fails.
    template <typename T>
    static std::shared_ptr<T> CreateObject(const std::string &type_name) {
        return Util::CastToDerived<T>(CreateObjectOfType(type_name));
    }

    /// Clears the registry. This is primarily for unit tests.
    static void Clear() { map_.clear(); }

  private:
    /// Stores the association between type names and creation functions.
    static std::unordered_map<std::string, CreationFunc> map_;

    /// Implements the AddType_() function using the name of the type.
    static void AddType_(const std::string &type_name,
                         const CreationFunc &creation_func);
};

}  // namespace Parser
