#include "Math/TextUtils.h"

#include "Math/PolygonBuilder.h"
#include "Util/FontSystem.h"

StrVec GetAvailableFontNames() {
    return FontSystem::GetInstalled()->GetAvailableFontNames();
}

bool IsValidFontName(const Str &font_name) {
    return FontSystem::GetInstalled()->IsValidFontName(font_name);
}

bool IsValidStringForFont(const Str &font_name, const Str &str, Str &reason) {
    return FontSystem::GetInstalled()->IsValidStringForFont(font_name, str,
                                                            reason);
}

FilePath GetFontPath(const Str &font_name) {
    return FontSystem::GetInstalled()->GetFontPath(font_name);
}

std::vector<Polygon> GetTextOutlines(const Str &font_name, const Str &text,
                                     float complexity, float char_spacing) {

    // Set up FontSystem outline building functions to use a PolygonBuilder.
    PolygonBuilder builder;
    FontSystem::GetInstalled()->GetTextOutlines(font_name, text, char_spacing,
                                                builder.SetUpForText());

    std::vector<Polygon> polys;
    builder.AddPolygons(polys, complexity);
    return polys;
}
