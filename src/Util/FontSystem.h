#pragma once

#include <functional>
#include <memory>

#include "Util/FilePath.h"

/// The FontSystem class is a singleton that manages system fonts. Most of
/// these functions are virtual to allow derived classes to redefine them for
/// testing.
///
/// \ingroup Utility
class FontSystem {
  public:
    /// This struct contains functions that handle processing of character
    /// outlines for GetTextOutlines(). An outline consists of some number of
    /// borders, each of which consists of a number of 2D points.
    struct OutlineFuncs {
        /// Begins an outline, given the nunber of borders in it.
        std::function<void (size_t)>             begin_outline_func;
        /// Begins a border, given the nunber of points in it.
        std::function<void (size_t)>             begin_border_func;
        /// Adds a point to the current border. The flag indicates whether the
        /// point is on the border (true) or is a control point for quadratic
        /// Bezier curves (false).
        std::function<void (float, float, bool)> add_point_func;
    };

    FontSystem();
    ~FontSystem();

    /// Returns a vector containing the names of all available fonts. This
    /// loads all of the known fonts.
    virtual StrVec GetAvailableFontNames() const;

    /// Returns true if the given name represents a valid font.
    virtual bool IsValidFontName(const Str &font_name) const;

    /// Returns true if the given string is valid for use with the named
    /// font. If this returns false, it sets \p reason to describe the problem.
    virtual bool IsValidStringForFont(const Str &font_name, const Str &str,
                                      Str &reason) const;

    /// Returns the path to the named font. The path will be empty if the font
    /// name is not valid.
    virtual FilePath GetFontPath(const Str &font_name) const;

    /// Computes character outlines for the given font, character string, and
    /// character spacing, invoking the OutlineFuncs functions for each.
    virtual void GetTextOutlines(const Str &font_name, const Str &text,
                                 float char_spacing,
                                 const OutlineFuncs &funcs) const;

  private:
    class Impl_;  // Does most of the work;
    std::unique_ptr<Impl_> impl_;
};
