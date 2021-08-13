#pragma once

#ifdef __GNUG__
#include <cxxabi.h>  // For demangling.
#endif

#include <assert.h>
#include <stdio.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include <magic_enum.hpp>

#include <ion/gfx/node.h>

#include "Flags.h"

// XXXX Break this up into Util/ directory files?

//! This namespace contains general utility functions and classes.
namespace Util {

// ----------------------------------------------------------------------------
// General utilities.
// ----------------------------------------------------------------------------

//! \name General Utilities
//!@{

//! This class can be used to temporarily disable \c stdout and \c stderr
//! output. Any code inside its scope will be redirected to /dev/null.
class OutputMuter {
  public:
    OutputMuter() {
        saved_stdout_ = *stdout;
        saved_stderr_ = *stderr;
        *stdout = *fopen("/dev/null", "w");
        *stderr = *fopen("/dev/null", "w");
    }
    ~OutputMuter() {
        *stdout = saved_stdout_;
        *stderr = saved_stderr_;
    }
  private:
    FILE saved_stdout_;
    FILE saved_stderr_;
};

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

// ----------------------------------------------------------------------------
// String utilities.
// ----------------------------------------------------------------------------

//! \name String Utilities
//!@{

//! Converts any printable type to a string.
template <typename T>
std::string ToString(const T &t) {
    std::ostringstream out;
    out << t;
    return out.str();
}

//! Returns true if two strings are equal, ignoring case.
bool StringsEqualNoCase(const std::string &s1, const std::string &s2);

//! Platform-specific C++ name demangling. For use with typeid(), for example.
std::string Demangle(const std::string &mangled_name);

//!@}

// ----------------------------------------------------------------------------
// Enum helpers.
// ----------------------------------------------------------------------------

//! \name Enumerated Type Helpers
//!@{

//! Converts an enum value to a string.
template <typename EnumType>
std::string EnumName(EnumType e) {
    return std::string(magic_enum::enum_name<EnumType>(e));
}

//! Converts an enum value to an int.
template <typename EnumType>
int EnumInt(EnumType e) {
    return static_cast<int>(magic_enum::enum_integer<EnumType>(e));
}

//! Accesses an enum value from a string. Returns false if the string does
//! represent a valid value.
template <typename EnumType>
bool EnumFromString(const std::string &s, EnumType &result) {
    auto val = magic_enum::enum_cast<EnumType>(s);
    if (val.has_value()) {
        result = val.value();
        return true;
    }
    return false;
}

//! Converts a Flags instance to a string of the form \c "kX|kY|...".
template <typename EnumType>
std::string EnumFlagNames(Flags<EnumType> flags) {
    std::string s = "";
    constexpr auto entries = magic_enum::enum_entries<EnumType>();
    for (auto &entry: entries) {
        if (flags.Has(entry.first)) {
            if (! s.empty())
                s += '|';
            s += std::string(entry.second);
        }
    }
    return s;
}

//!@}

// ----------------------------------------------------------------------------
// Time helpers.
// ----------------------------------------------------------------------------

//! \name Enumerated Type Helpers
//!@{

//! The Time class wraps std::filesystem::file_time_type to make it easier to
//! use.
class Time {
  public:
    //! The default constructor creates an undefined time.
    Time() {}

    //! The constructor wraps a file_time_type.
    Time(const std::filesystem::file_time_type &time) : time_(time) {}

    //! Constructs an instance representing the current time.
    static Time Now() {
        return Time(std::filesystem::file_time_type::clock::now());
    }

    //! Constructs an instance representing the last modification time of a
    //! file with the given path, which must exist.
    static Time ModTime(const std::string &path) {
        assert(std::filesystem::exists(path));
        return Time(std::filesystem::last_write_time(path));
    }

    //! Time comparisons.
    bool operator==(const Time &other) { return time_ == other.time_; }
    bool operator!=(const Time &other) { return time_ != other.time_; }
    bool operator<(const  Time &other) { return time_  <  other.time_; }
    bool operator>(const  Time &other) { return time_  >  other.time_; }
    bool operator<=(const Time &other) { return time_ <= other.time_; }
    bool operator>=(const Time &other) { return time_ >= other.time_; }

    //! Allows output of Time values.
    std::string ToString() const {
        auto sctp =
            std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                time_ - std::filesystem::file_time_type::clock::now()
                + std::chrono::system_clock::now());
        std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
        return std::asctime(std::localtime(&cftime));
    }

  private:
    //! Wrapped std::filesystem::file_time_type.
    std::filesystem::file_time_type time_;
};

//! Allows output of Time values.
inline std::ostream & operator<<(std::ostream &out, const Time &t) {
    return out << t.ToString();
}

//!}

// ----------------------------------------------------------------------------
// Ion graph utilities.
// ----------------------------------------------------------------------------

//! \name Ion Graph Utilities
//!@{

//! Typedef for a path from a Node to a descendent node.
typedef std::vector<ion::gfx::NodePtr> NodePath;

//! Searches under the given root node for a node with the given name
//! (label). If found, this sets path to contain the path of nodes from the
//! root to it, inclusive, and returns true. Otherwise, it just returns false.
bool SearchForNode(const ion::gfx::NodePtr &root,
                   const std::string &name, NodePath &path);

//!@}

}  // namespace Util
