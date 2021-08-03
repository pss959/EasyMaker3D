#pragma once

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

    //! Returns true if the given flag bit is set.
    bool Has(EnumClass flag) const {
        return flags_ & static_cast<uint32_t>(flag);
    }

  private:
    uint32_t flags_ = 0;
};
