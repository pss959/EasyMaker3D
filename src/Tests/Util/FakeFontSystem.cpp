//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tests/Util/FakeFontSystem.h"

#include "Util/General.h"

StrVec FakeFontSystem::GetAvailableFontNames() const {
    // This should to be kept in sync with resources/fonts if it ever changes.
    return StrVec{
        "Arial-Bold",
        "Arial-Regular",
        "DejaVu Sans Mono-Book",
        "Verdana-Bold",
        "Verdana-Bold-Italic",
        "Verdana-Italic",
        "Verdana-Regular",
    };
}

bool FakeFontSystem::IsValidFontName(const Str &font_name) const {
    return Util::Contains(GetAvailableFontNames(), font_name);
}

bool FakeFontSystem::IsValidStringForFont(const Str &font_name, const Str &str,
                                          Str &reason) const {
    if (! IsValidFontName(font_name)) {
        reason = "Invalid font";
        return false;
    }

    if (str.empty()) {
        reason = "Empty string";
        return false;
    }

    const Str valid =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";
    if (str.find_first_not_of(valid) != std::string::npos) {
        reason = "Invalid character";
        return false;
    }

    return true;
}

FilePath FakeFontSystem::GetFontPath(const Str &font_name) const {
    return IsValidFontName(font_name) ?
        FilePath("/fonts/" + font_name + ".ttf") : FilePath();
}

void FakeFontSystem::GetTextOutlines(const Str &font_name, const Str &text,
                                     float char_spacing,
                                     const OutlineFuncs &funcs) const {
    // Create a 1x2 rectangle for each character. Note that this has to be
    // clockwise to be consistent with what PolygonBuilder expects (because of
    // Freetype2).
    float x = 0;
    for (size_t i = 0; i < text.size(); ++i) {
        funcs.begin_outline_func(1);
        funcs.begin_border_func(4);
        funcs.add_point_func(x,     0, true);
        funcs.add_point_func(x,     2, true);
        funcs.add_point_func(x + 1, 2, true);
        funcs.add_point_func(x + 1, 0, true);
        x += char_spacing;
    }
}
