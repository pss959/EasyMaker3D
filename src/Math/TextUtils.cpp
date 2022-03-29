#include "Math/TextUtils.h"

// Freetype2 headers.
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "Math/PolygonBuilder.h"
#include "Util/Assert.h"

namespace {

// ----------------------------------------------------------------------------
// Internal FontManager_ class.
// ----------------------------------------------------------------------------

class FontManager_ {
  public:
    typedef FT_Face Font;

    /// If the font with the given path can be loaded, this returns true and
    /// sets font to refer to it. Otherwise, it just returns false. This uses a
    /// map to avoid loading the same font multiple times.
    bool LoadFont(const FilePath &path, Font &font);

    /// Returns a string descriptor for the given font.
    std::string GetFontDesc(Font font) {
        return std::string(font->family_name) + "-" + font->style_name;
    }

    /// Adds polygons representing the glyph for the given character in the
    /// given font to the vector and returns the X advance amount for the
    /// glyph. The starting X value and the complexity (0-1) for the glyph is
    /// provided.
    float AddGlyphPolygons(Font font, char c, float x_start, float complexity,
                           std::vector<Polygon> &polys);

  private:
    typedef std::unordered_map<FilePath, Font> FontMap_;

    FT_Library library_;
    bool       library_initialized_ = false;
    FontMap_   font_map_;

    void InitLibrary_();
    bool CanLoadFace_(FT_Face face);

    /// Converts from Q26.6 fixed format to a float.
    static float FromQ26_6_(FT_Pos n) { return n / 64.f; }

    /// Converts from Q16.16 fixed format to a float.
    static float FromQ16_16_(FT_Pos n) { return n / 65536.f; }
};

static FontManager_ s_font_manager_;

bool FontManager_::LoadFont(const FilePath &path, Font &font) {
    if (! library_initialized_)
        InitLibrary_();

    // Try the map first.
    const auto it = font_map_.find(path);
    if (it != font_map_.end()) {
        font = it->second;
        return true;
    }

    // Create a new font face and add it to the map.
    if (FT_New_Face(library_, path.ToString().c_str(), 0, &font) != FT_Err_Ok)
        return false;
    font_map_[path] = font;

    // See if the face can be loaded.
    return CanLoadFace_(font);
}

float FontManager_::AddGlyphPolygons(Font font, char c, float x_start,
                                     float complexity,
                                     std::vector<Polygon> &polys) {
    // Get the glyph.
    FT_UInt glyph_index = FT_Get_Char_Index(font, c);
    FT_Load_Glyph(font, glyph_index, FT_LOAD_NO_SCALE);
    FT_Glyph glyph;
    FT_Get_Glyph(font->glyph, &glyph);

    // Get the glyph outline.
    const FT_OutlineGlyph outline_glyph =
        reinterpret_cast<FT_OutlineGlyph>(glyph);
    FT_Outline *outline = &outline_glyph->outline;

    PolygonBuilder builder;
    builder.BeginOutline(outline->n_contours);

    int cur_point = 0;
    for (int i = 0; i < outline->n_contours; ++i) {
        // Begin the contour.
        const int contour_end = outline->contours[i];
        const int point_count = contour_end - cur_point + 1;
        builder.BeginBorder(point_count);

        for (; cur_point <= contour_end; ++cur_point) {
            const FT_Vector &pt     = outline->points[cur_point];
            const unsigned int tags = outline->tags[cur_point];
            const bool is_on_curve  = FT_CURVE_TAG(tags) == FT_Curve_Tag_On;

            // Point coordinates are in Q26.6 format.
            const Point2f p(FromQ26_6_(pt.x), FromQ26_6_(pt.y));
            builder.AddPoint(p, is_on_curve);
        }
    }

    builder.AddPolygons(polys, complexity);

    // Advance is in Q16.16 fixed format.
    return FromQ16_16_(glyph->advance.x);
}

void FontManager_::InitLibrary_() {
    ASSERT(! library_initialized_);
    if (FT_Init_FreeType(&library_) != FT_Err_Ok)
        ASSERTM(false, "Unable to initialize FreeType2");
    library_initialized_ = true;
}

bool FontManager_::CanLoadFace_(FT_Face face) {
    // Load the glyph for 'A'.
    FT_UInt glyph_index = FT_Get_Char_Index(face, 'A');
    if (FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_SCALE) != FT_Err_Ok)
        return false;

    FT_Glyph glyph;
    if (FT_Get_Glyph(face->glyph, &glyph) != FT_Err_Ok)
        return false;

    if (glyph->format != FT_GLYPH_FORMAT_OUTLINE)
        return false;

    return true;
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

std::string GetFontDesc(const FilePath &path) {
    FontManager_::Font font;
    return s_font_manager_.LoadFont(path, font) ?
        s_font_manager_.GetFontDesc(font) : "";
}

std::vector<Polygon> GetTextOutlines(const FilePath &path,
                                     const std::string &text,
                                     float complexity,
                                     float char_spacing) {
    std::vector<Polygon> polygons;

    FontManager_::Font font;
    if (s_font_manager_.LoadFont(path, font)) {
        // Process each character in the text.
        float x = 0;
        for (const char c: text) {
            const float advance = s_font_manager_.AddGlyphPolygons(
                font, c, x, complexity, polygons);
            x += char_spacing * advance;
        }
    }

    return polygons;
}
