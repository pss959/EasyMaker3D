#pragma once

#include "Util/FontSystem.h"

/// FakeFontSystem is a derived FontSystem class that allows for testing
/// font-related functions without relying on specific system fonts.
///
/// \ingroup Tests
class FakeFontSystem : public FontSystem {
  public:
    /// Returns a vector containing "FontA", "FontB", and "FontC".
    virtual StrVec GetAvailableFontNames() const override;

    /// Returns true if the given name is in the above list.
    virtual bool IsValidFontName(const Str &font_name) const override;

    /// Returns true if the font is valid and the string is not empty and
    /// contains only letters (no numbers, spaces or special characters). If
    /// this returns false, it sets \p reason to "Invalid font", "Empty
    /// string", or "Invalid character".
    virtual bool IsValidStringForFont(const Str &font_name, const Str &str,
                                      Str &reason) const override;

    /// If the font is valid, this returns the path "/fonts/<font_name>.ttf".
    /// If not, this returns an empty path.
    virtual FilePath GetFontPath(const Str &font_name) const override;

    /// Creates a constant rectangle outline for each character.
    virtual void GetTextOutlines(const Str &font_name, const Str &text,
                                 float char_spacing,
                                 const OutlineFuncs &funcs) const override;
};
