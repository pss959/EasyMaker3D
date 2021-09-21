#pragma once

#include <algorithm>
#include <memory>

//! This namespace contains general utility functions and classes.
namespace Util {

//! \name General Utilities
//!@{

//! Convenience to cast an std::shared_ptr of a derived class to an
//! std::shared_ptr of a base class.
template <typename Base, typename Derived>
std::shared_ptr<Base> CastToBase(const std::shared_ptr<Derived> &ptr) {
    return std::dynamic_pointer_cast<Base>(ptr);
}

//! Convenience to cast an std::shared_ptr of a base class to an
//! std::shared_ptr of a derived class.
template <typename Derived, typename Base>
std::shared_ptr<Derived> CastToDerived(const std::shared_ptr<Base> &ptr) {
    return std::dynamic_pointer_cast<Derived>(ptr);
}

//! Convenience that returns true if the given std::shared_ptr holds a pointer
//! to an instance of the given class.
template <typename Want, typename Have>
bool IsA(const std::shared_ptr<Have> &ptr) {
    return std::dynamic_pointer_cast<Want>(ptr).get();
}

//! Returns true if an STL container contains the given element.
template <typename Container, typename Element>
bool Contains(const Container &ctr, const Element &elt) {
    return std::find(ctr.begin(), ctr.end(), elt) != ctr.end();
}

//! Returns true if an STL map or set contains the given element
template <typename Map, typename Element>
bool MapContains(const Map &map, const Element &elt) {
    return map.find(elt) != map.end();
}

//! Converts a vector of elements of type From to a vector of elements of type
//! To using the given conversion function.
template <typename To, typename From>
std::vector<To> ConvertVector(const std::vector<From> &from_vec,
                              const std::function<To(const From &)> &func) {
    std::vector<To> to_vec;
    to_vec.reserve(from_vec.size());
    std::transform(from_vec.begin(), from_vec.end(),
                   std::back_inserter(to_vec), func);
    return to_vec;
}

//! Appends a vector of elements to another vector of the same type.
template <typename T>
void AppendVector(const std::vector<T> &from, std::vector<T> &to) {
    to.insert(to.end(), from.begin(), from.end());
}

//!@}

}  // namespace Util
