#include "Math/TextUtils.h"

#include "Math/PolygonBuilder.h"
#include "Util/FontSystem.h"

static std::shared_ptr<FontSystem> s_font_system_(new FontSystem);

void InstallFontSystem(const std::shared_ptr<FontSystem> &fs) {
    s_font_system_ = fs;
}

StrVec GetAvailableFontNames() {
    return s_font_system_->GetAvailableFontNames();
}

bool IsValidFontName(const Str &font_name) {
    return s_font_system_->IsValidFontName(font_name);
}

bool IsValidStringForFont(const Str &font_name, const Str &str, Str &reason) {
    return s_font_system_->IsValidStringForFont(font_name, str, reason);
}

FilePath GetFontPath(const Str &font_name) {
    return s_font_system_->GetFontPath(font_name);
}

std::vector<Polygon> GetTextOutlines(const Str &font_name, const Str &text,
                                     float complexity, float char_spacing) {

    // Set up FontSystem outline building functions to use a PolygonBuilder.
    PolygonBuilder builder;
    FontSystem::OutlineFuncs funcs;
    funcs.begin_outline_func = [&](size_t nc){ builder.BeginOutline(nc); };
    funcs.begin_border_func  = [&](size_t np){ builder.BeginBorder(np);  };
    funcs.add_point_func     = [&](float x, float y, bool is_on_curve){
        builder.AddPoint(Point2f(x, y), is_on_curve); };

    s_font_system_->GetTextOutlines(font_name, text, char_spacing, funcs);

    std::vector<Polygon> polys;
    builder.AddPolygons(polys, complexity);
    return polys;
}
