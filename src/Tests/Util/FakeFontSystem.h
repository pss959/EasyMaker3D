#pragma once

#include "Util/FontSystem.h"

/// FakeFontSystem is a derived FontSystem class that allows for testing
/// font-related functions without relying on specific system fonts.
///
/// \ingroup Tests
class FakeFontSystem : public FontSystem {
  public:
    /// Redefines this to return true;
    virtual bool IsFake() const override { return true; }

    /// Returns a vector containing all the current names of fonts in the
    /// resources/fonts directory.
    virtual StrVec GetAvailableFontNames() const override;

    /// Returns true if the given name is in GetAvailableFontNames().
    virtual bool IsValidFontName(const Str &font_name) const override;

    /// Returns true if the font is valid and the string is not empty and
    /// contains only letters, numbers, or spaces. If this returns false, it
    /// sets \p reason to "Invalid font", "Empty string", or "Invalid
    /// character".
    virtual bool IsValidStringForFont(const Str &font_name, const Str &str,
                                      Str &reason) const override;

    /// Creates a constant rectangle outline for each character.
    virtual void GetTextOutlines(const Str &font_name, const Str &text,
                                 float char_spacing,
                                 const OutlineFuncs &funcs) const override;
};
