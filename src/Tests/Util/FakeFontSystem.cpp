#include "Tests/Util/FakeFontSystem.h"

StrVec FakeFontSystem::GetAvailableFontNames() const {
    return StrVec{ "FontA", "FontB", "FontC" };
}

bool FakeFontSystem::IsValidFontName(const Str &font_name) const {
    return font_name == "FontA" || font_name == "FontB" ||
        font_name == "FontC";
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

    const Str letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (str.find_first_not_of(letters) != std::string::npos) {
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
    // XXXX
}
