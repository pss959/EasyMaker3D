//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Math/Polygon.h"
#include "Util/FilePath.h"

/// \file
/// This file defines functions for doing math with fonts and text.
///
/// \ingroup Math

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
