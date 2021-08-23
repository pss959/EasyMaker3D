#pragma once

#include <magic_enum.hpp>

namespace Util {

//! \name Enumerated Type Helpers
//!@{

//! The Flags class is a wrapper for an enum class representing bitwise flags.
//! The enum is passed as the template parameter.
template <typename EnumClass>
class Flags {
  public:
    Flags()  {}
    ~Flags() {}

    //! Sets the given flag in an instance.
    void Set(EnumClass flag) {
        flags_ |= static_cast<uint32_t>(flag);
    }

    //! Sets all flags to true or false.
    void SetAll(bool b) {
        if (b) {
            for (auto f: magic_enum::enum_values<EnumClass>())
                Set(f);
        }
        else {
            flags_ = 0;
        }
    }

    //! Resets the given flag in an instance.
    void Reset(EnumClass flag) {
        flags_ &= ~static_cast<uint32_t>(flag);
    }

    //! Returns true if the given flag bit is set.
    bool Has(EnumClass flag) const {
        return flags_ & static_cast<uint32_t>(flag);
    }

    //! Returns true if any flag is set.
    bool HasAny() const {
        return flags_;
    }

    //! Returns true if any of the flags overlap.
    bool HasAnyFrom(const Flags<EnumClass> &other_flags) const {
        return flags_ & other_flags.flags_;
    }

  private:
    uint32_t flags_ = 0;
};

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

}  // namespace Util
