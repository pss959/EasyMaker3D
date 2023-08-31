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

#include "Util/Assert.h" // XXXX
void FakeFontSystem::GetTextOutlines(const Str &font_name, const Str &text,
                                     float char_spacing,
                                     const OutlineFuncs &funcs) const {
    // XXXX
    ASSERTM(false, "FakeFontSystem::GetTextOutlines() called");
}
