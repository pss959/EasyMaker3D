#pragma once

#include <ion/base/stringutils.h>

#include "Util/Enum.h"

namespace Util {

/// The Flags class is a wrapper for an enum class representing bitwise flags.
/// The enum is passed as the template parameter.
///
/// \ingroup Utility
template <typename EnumClass>
class Flags {
  public:
    using FlagsType = Flags<EnumClass>;

    Flags()  {}
    ~Flags() {}

    /// Sets the given flag in an instance.
    void Set(EnumClass flag) {
        flags_ |= static_cast<uint32_t>(flag);
    }

    /// Sets all flags to true or false.
    void SetAll(bool b) {
        if (b) {
            for (auto f: Util::EnumValuesArray<EnumClass>())
                Set(f);
        }
        else {
            flags_ = 0;
        }
    }

    /// Resets the given flag in an instance.
    void Reset(EnumClass flag) {
        flags_ &= ~static_cast<uint32_t>(flag);
    }

    /// Returns true if the given flag bit is set.
    bool Has(EnumClass flag) const {
        return flags_ & static_cast<uint32_t>(flag);
    }

    /// Returns true if only the given flag bit is set.
    bool HasOnly(EnumClass flag) const {
        return flags_ == static_cast<uint32_t>(flag);
    }

    /// Returns true if any flag is set.
    bool HasAny() const {
        return flags_;
    }

    /// Returns true if any of the flags overlap.
    bool HasAnyFrom(const FlagsType &other_flags) const {
        return flags_ & other_flags.flags_;
    }

    /// Adds a flag to an existing instance and returns the result.
    FlagsType Add(EnumClass flag) const {
        Flags f = *this;
        f.Set(flag);
        return f;
    }

    /// Removes a flag to an existing instance and returns the result.
    FlagsType Remove(EnumClass flag) const {
        Flags f = *this;
        f.Reset(flag);
        return f;
    }

    /// Converts the value to a string formed by putting a bitwise-or '|'
    /// between all set flags.
    Str ToString() const {
        Str s;
        for (auto f: Util::EnumValuesArray<EnumClass>()) {
            if (Has(f)) {
                if (! s.empty())
                    s += '|';
                s += EnumName(f);
            }
        }
        return s;
    }

    /// Parses a string in the format returned by ToString, setting the
    /// corresponding flags in the given instance. Returns false on error.
    static bool FromString(const Str &str, FlagsType &flags) {
        EnumClass e;
        for (const Str &word: ion::base::SplitString(str, "|")) {
            const Str &es = ion::base::TrimStartAndEndWhitespace(word);
            if (! EnumFromString<EnumClass>(es, e))
                return false;
            flags.Set(e);
        }
        return true;
    }

    /// Equality operator.
    bool operator==(const FlagsType &f) const = default;

  private:
    uint32_t flags_ = 0;
};

}  // namespace Util
