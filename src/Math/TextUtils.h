#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Math/Polygon.h"
#include "Util/FilePath.h"

class FontSystem;

/// \file
/// This file defines functions for doing math with fonts and text.
///
/// \ingroup Math

/// Sets a FontSystem instance to use for all font-related functions. This
/// allows a derived version to be installed for testing. An instance of the
/// base FontSystem class is installed by default.
void InstallFontSystem(const std::shared_ptr<FontSystem> &fs);

/// Returns a vector containing the names of all available fonts. This loads
/// all of the known fonts.
StrVec GetAvailableFontNames();

/// Returns true if the given name represents a valid font.
bool IsValidFontName(const Str &font_name);

/// Returns true if the the given string is valid for use with the named
/// font. If this returns false, it sets reason to describe the problem.
bool IsValidStringForFont(const Str &font_name, const Str &str, Str &reason);

/// Returns the path to the named font. The path will be empty if the font name
/// is not valid.
FilePath GetFontPath(const Str &font_name);

/// Computes and returns a vector of Polygon instances representing the
/// character outlines for the given font, character string, character spacing,
/// and complexity.
std::vector<Polygon> GetTextOutlines(const Str &font_name, const Str &text,
                                     float complexity, float char_spacing);
