#pragma once

#include <string>
#include <vector>

#include "Math/Polygon.h"
#include "Util/FilePath.h"

/// \name Text Utilities
///@{

/// Returns a vector containing the names of all available fonts. This loads
/// all of the known fonts.
std::vector<std::string> GetAvailableFontNames();

/// Returns true if the given name represents a valid font.
bool IsValidFontName(const std::string &font_name);

/// Returns the path to the named font. The path will be empty if the font name
/// is not valid.
FilePath GetFontPath(const std::string &font_name);

/// Computes and returns a vector of Polygon instances representing the
/// character outlines for the given font, character string, character spacing,
/// and complexity.
std::vector<Polygon> GetTextOutlines(const std::string &font_name,
                                     const std::string &text,
                                     float complexity,
                                     float char_spacing);

///@}
