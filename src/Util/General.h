#pragma once

#include <algorithm>
#include <functional>
#include <memory>

/// \file
/// This file contains general utility functions for simplifying and clarifying
/// code.
///
/// \ingroup Utility

/// This namespace contains general utility functions and classes.
namespace Util {

/// Enum indicating application status. This is used to determine whether code
/// is running inside a unit test or simulation test.
enum class AppType {
    kInteractive,  ///< Regular interactive application (default).
    kUnitTest,     ///< Non-interactive unit test.
    kSimTest,      ///< Interactive simulation unit test.
};

/// Enum indicating application status. This is used to determine whether code
/// is running inside a unit test or simulation test. This is initialized to
/// AppType::kInteractive and is changed at the beginning of unit tests.
extern AppType app_type;

/// Flag indicating whether code is running inside the main application. It is
/// initialized to false and is set to true at the beginning of the main app.
extern bool is_in_main_app;

/// Convenience that returns true if the given std::shared_ptr holds a pointer
/// to an instance of the given class.
template <typename Want, typename Have>
bool IsA(const std::shared_ptr<Have> &ptr) {
    return std::dynamic_pointer_cast<Want>(ptr).get();
}

/// Returns true if an STL container contains the given element.
template <typename Container, typename Element>
bool Contains(const Container &ctr, const Element &elt) {
    return std::find(ctr.begin(), ctr.end(), elt) != ctr.end();
}

/// Returns a vector containing all keys from a map.
template <typename Map>
std::vector<typename Map::key_type> GetKeys(const Map &map) {
    std::vector<typename Map::key_type> keys;
    keys.reserve(map.size());
    std::transform(map.begin(), map.end(),
                   std::back_inserter(keys),
                   [](auto &pair){ return pair.first; });
    return keys;
}

/// Returns a vector containing all values from a map.
template <typename Map>
std::vector<typename Map::mapped_type> GetValues(const Map &map) {
    std::vector<typename Map::mapped_type> values;
    values.reserve(map.size());
    std::transform(map.begin(), map.end(),
                   std::back_inserter(values),
                   [](auto &pair){ return pair.second; });
    return values;
}

/// Finds all elements in a vector that satisfy the given predicate function
/// and returns a vector of the results.
template <typename T>
std::vector<T> FindAll(const std::vector<T> &vec,
                       const std::function<bool(const T &)> &func) {
    std::vector<T> results;
    std::copy_if(vec.begin(), vec.end(), std::back_inserter(results), func);
    return results;
}

/// Converts a vector of elements of type From to a vector of elements of type
/// To using the given conversion function.
template <typename To, typename From>
std::vector<To> ConvertVector(const std::vector<From> &from_vec,
                              const std::function<To(const From &)> &func) {
    std::vector<To> to_vec;
    to_vec.reserve(from_vec.size());
    std::transform(from_vec.begin(), from_vec.end(),
                   std::back_inserter(to_vec), func);
    return to_vec;
}

/// Appends a vector of elements to another vector of the same type.
template <typename T>
void AppendVector(const std::vector<T> &from, std::vector<T> &to) {
    to.insert(to.end(), from.begin(), from.end());
}

/// Creates a shared_ptr for the given pointer of the templated type that does
/// not delete the item when the shared_ptr is destroyed. This can be used to
/// create temporary shared pointers.
template <typename T> std::shared_ptr<T> CreateTemporarySharedPtr(T *ptr) {
    return std::shared_ptr<T>(std::shared_ptr<T>{}, ptr);
}

}  // namespace Util
