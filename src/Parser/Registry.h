//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <functional>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "Parser/Object.h"
#include "Util/Assert.h"

namespace Parser {

/// The Parser::Registry class maintains a registry of concrete classes derived
/// from Parser::Object. Each entry in the registry is keyed by the name of the
/// class and contains a function that can be invoked to create an instance of
/// that class.
///
/// This is essentially a singleton since all functions and storage are static.
///
/// \ingroup Parser
class Registry {
  public:
    /// Convenience alias for the function used to create an object.
    using CreationFunc = std::function<Object *()>;

    /// Registers a derived concrete Parser::Object class that can be parsed.
    /// The unique name to use for the class and a function that can be used to
    /// create an instance are supplied.
    template <typename T>
    static void AddType(const Str &type_name) {
        // Make sure the class does not have a public default constructor.
        ASSERTM(! std::is_default_constructible<T>::value,
                "Class " + type_name + " has a public constructor");
        AddType_(type_name, typeid(T), []{ return new T; });
    }

    /// Calls the CreationFunc for the given object type, sets up the resulting
    /// instance, and returns it. Throws an Exception if the type was not
    /// registered. The new object is assigned the given name. The new object
    /// is assumed to be complete unless false is passed for is_complete.
    static ObjectPtr CreateObjectOfType(const Str &type_name,
                                        const Str &name = "",
                                        bool is_complete = true) {
        return CreateObjectOfType_(type_name, name, is_complete);
    }

    /// Convenience that uses CreateObjectOfType() to create an object and then
    /// casts it to the target type. Throws an Exception if the type was not
    /// registered. Returns a null pointer if the cast fails. The new object is
    /// assigned the given name.
    template <typename T>
    static std::shared_ptr<T> CreateObject(const Str &name = "") {
        const Str &type_name = FindTypeName_(typeid(T));
        return std::dynamic_pointer_cast<T>(
            CreateObjectOfType_(type_name, name, true));
    }

    /// Clears the registry. This is primarily for unit tests.
    static void Clear() {
        creation_map_.clear();
        type_name_map_.clear();
    }

    /// Returns the number of registered type names.
    static size_t GetTypeNameCount() { return creation_map_.size(); }

    /// Returns a vector containing all registered type names.
    static StrVec GetAllTypeNames();

  private:
    /// Convenience alias for the map from typeid to type name.
    using TypeNameMap_ = std::unordered_map<std::type_index, Str>;

    /// Convenience alias for the map from type name to creation function.
    using CreationMap_ = std::unordered_map<Str, CreationFunc>;

    /// Stores the association between C++ typeid and type names.
    static TypeNameMap_ type_name_map_;

    /// Stores the association between type names and creation functions.
    static CreationMap_ creation_map_;

    /// Implements the AddType_() function using the name of the type.
    static void AddType_(const Str &type_name, const std::type_info &info,
                         const CreationFunc &creation_func);

    /// Returns the name of the type with the given info. Throws an exception
    /// if it is not found.
    static Str FindTypeName_(const std::type_info &info);

    /// This is used by both CreateObjectOfType() and
    /// CreateObjectForParsing_(). If is_complete is true, CreationDone() is
    /// called for the new instance.
    static ObjectPtr CreateObjectOfType_(const Str &type_name, const Str &name,
                                         bool is_complete);

    // Allow Object and Parser to call CreateObjectOfType_().
    friend class Object;
    friend class Parser;
};

}  // namespace Parser
