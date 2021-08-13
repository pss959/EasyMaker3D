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
    return std::shared_ptr<Base>(ptr.get());
}

//! Returns true if an STL container contains the given element.
template <typename Container, typename Element>
bool Contains(const Container &ctr, const Element &elt) {
    return std::find(ctr.begin(), ctr.end(), elt) != ctr.end();
}

//! Returns true if an STL map contains the given element
template <typename Map, typename Element>
bool MapContains(const Map &map, const Element &elt) {
    return map.find(elt) != map.end();
}

//!@}

}  // namespace Util
