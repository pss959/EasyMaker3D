#pragma once

#include <string>
#include <vector>

#include "Math/Polygon.h"
#include "Util/FilePath.h"

/// \name Text Utilities
///@{

/// Returns a string descriptor for the font with the given path. If the path
/// does not refer to a valid font, this returns an empty string.
std::string GetFontDesc(const FilePath &path);

/// Computes and returns a vector of Polygon instances representing the
/// character outlines for the given font path, character string, character
/// spacing, and complexity.
std::vector<Polygon> GetTextOutlines(const FilePath &path,
                                     const std::string &text,
                                     float complexity,
                                     float char_spacing);

///@}
