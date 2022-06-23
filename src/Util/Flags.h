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
    bool HasAnyFrom(const Flags<EnumClass> &other_flags) const {
        return flags_ & other_flags.flags_;
    }

    /// Converts the value to a string formed by putting a bitwise-or '|'
    /// between all set flags.
    std::string ToString() const {
        std::string s;
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
    static bool FromString(const std::string &str, Flags<EnumClass> &flags) {
        EnumClass e;
        for (const std::string &word: ion::base::SplitString(str, "|")) {
            const std::string &es = ion::base::TrimStartAndEndWhitespace(word);
            if (! EnumFromString<EnumClass>(es, e))
                return false;
            flags.Set(e);
        }
        return true;
    }

  private:
    uint32_t flags_ = 0;
};

}  // namespace Util
