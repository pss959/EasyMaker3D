#include "Math/TextUtils.h"

// Freetype2 headers.
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <algorithm>
#include <unordered_map>

#include "Math/PolygonBuilder.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/KLog.h"

namespace {

// ----------------------------------------------------------------------------
// Internal FontManager_ class. A static instance of this is used for all
// public functions.
// ----------------------------------------------------------------------------

class FontManager_ {
  public:
    // Each of these implements the public function with the same name.
    std::vector<std::string> GetAvailableFontNames();
    bool IsValidFontName(const std::string &font_name);
    bool IsValidStringForFont(const std::string &font_name,
                              const std::string &str, std::string &reason);
    FilePath GetFontPath(const std::string &font_name);
    std::vector<Polygon> GetTextOutlines(const std::string &font_name,
                                         const std::string &text,
                                         float complexity,
                                         float char_spacing);

  private:
    typedef std::unordered_map<std::string, FilePath> PathMap_;
    typedef std::unordered_map<std::string, FT_Face>  FaceMap_;

    bool     is_initialized_ = false;
    PathMap_ path_map_;  ///< Maps font names to FilePath instances.
    FaceMap_ face_map_;  ///< Maps font names to FT_Face instances.

    /// Loads all fonts found in the fonts directory. This must be called
    /// before any other functions. It is safe to call multiple times.
    void Init_();

    bool CanLoadFace_(FT_Face face);

    /// Adds polygons representing the glyph for the given character in the
    /// given font to the vector and returns the X advance amount for the
    /// glyph. The starting X value and the complexity (0-1) for the glyph is
    /// provided.
    float AddGlyphPolygons_(FT_Face face, char c, float x_start,
                            float complexity, std::vector<Polygon> &polys);

    /// Converts from Q26.6 fixed format to a float.
    static float FromQ26_6_(FT_Pos n) { return n / 64.f; }

    /// Converts from Q16.16 fixed format to a float.
    static float FromQ16_16_(FT_Pos n) { return n / 65536.f; }
};

static FontManager_ s_font_manager_;

std::vector<std::string> FontManager_::GetAvailableFontNames() {
    Init_();
    std::vector<std::string> names = Util::GetKeys(face_map_);
    std::sort(names.begin(), names.end());
    return names;
}

bool FontManager_::IsValidFontName(const std::string &font_name) {
    Init_();
    return Util::MapContains(face_map_, font_name);
}

bool FontManager_::IsValidStringForFont(const std::string &font_name,
                                        const std::string &str,
                                        std::string &reason) {
    Init_();
    if (str.empty()) {
        reason = "Empty string";
        return false;
    }

    const auto it = face_map_.find(font_name);
    if (it == face_map_.end()) {
        reason = "Invalid font name: " + font_name;
        return false;
    }

    // Make sure each character appears in the font face.
    FT_Face face = it->second;
    bool any_non_space = false;
    std::string bad_chars;
    for (auto c: str) {
        if (! std::isspace(c))
            any_non_space = true;
        if (FT_Get_Char_Index(face, c) == 0)
            bad_chars += c;
    }
    if (! any_non_space) {
        reason = "String has only space characters";
        return false;
    }
    if (! bad_chars.empty()) {
        reason = "String contains invalid character(s) for the font: [" +
            bad_chars + "]";
        return false;
    }

    reason.clear();
    return true;
}

FilePath FontManager_::GetFontPath(const std::string &font_name) {
    Init_();
    const auto it = path_map_.find(font_name);
    return it == path_map_.end() ? FilePath() : it->second;
}

std::vector<Polygon> FontManager_::GetTextOutlines(const std::string &font_name,
                                                   const std::string &text,
                                                   float complexity,
                                                   float char_spacing) {
    Init_();
    std::vector<Polygon> polygons;
    const auto it = face_map_.find(font_name);
    if (it != face_map_.end()) {
        const auto &face = it->second;
        // Process each character in the text.
        float x = 0;
        for (const char c: text) {
            const float advance =
                AddGlyphPolygons_(face, c, x, complexity, polygons);
            x += char_spacing * advance;
        }
    }
    return polygons;
}

void FontManager_::Init_() {
    if (is_initialized_)
        return;

    // Initialize the Freetype2 library.
    FT_Library lib;
    if (FT_Init_FreeType(&lib) != FT_Err_Ok)
        ASSERTM(false, "Unable to initialize FreeType2");

    // Access all font files.
    const FilePath dir_path = FilePath::GetResourcePath("fonts", FilePath());
    std::vector<std::string> subdirs;
    std::vector<std::string> files;
    dir_path.GetContents(subdirs, files, ".ttf", false);

    for (const auto &f: files) {
        const FilePath path = FilePath::Join(dir_path, f);

        // Create a new font face and make sure it can be loaded. If so, add it
        // to the maps.
        FT_Face face;
        if (FT_New_Face(lib, path.ToString().c_str(), 0, &face) == FT_Err_Ok &&
            CanLoadFace_(face)) {
            const std::string name =
                std::string(face->family_name) + "-" + face->style_name;
            KLOG('z', "Loaded font '" << name << " from path '"
                 << path.ToString() << "'");
            ASSERTM(! Util::MapContains(face_map_, name), name);
            path_map_[name] = path;
            face_map_[name] = face;
        }
    }

    is_initialized_ = true;
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

float FontManager_::AddGlyphPolygons_(FT_Face face, char c, float x_start,
                                     float complexity,
                                      std::vector<Polygon> &polys) {
    // Get the glyph.
    FT_UInt glyph_index = FT_Get_Char_Index(face, c);
    FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_SCALE);
    FT_Glyph glyph;
    FT_Get_Glyph(face->glyph, &glyph);

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
            const Point2f p(x_start + FromQ26_6_(pt.x), FromQ26_6_(pt.y));
            builder.AddPoint(p, is_on_curve);
        }
    }

    builder.AddPolygons(polys, complexity);

    // Advance is in Q16.16 fixed format.
    return FromQ16_16_(glyph->advance.x);
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

std::vector<std::string> GetAvailableFontNames() {
    return s_font_manager_.GetAvailableFontNames();
}

bool IsValidFontName(const std::string &font_name) {
    return s_font_manager_.IsValidFontName(font_name);
}

bool IsValidStringForFont(const std::string &font_name, const std::string &str,
                          std::string &reason) {
    return s_font_manager_.IsValidStringForFont(font_name, str, reason);
}

FilePath GetFontPath(const std::string &font_name) {
    return s_font_manager_.GetFontPath(font_name);
}

std::vector<Polygon> GetTextOutlines(const std::string &font_name,
                                     const std::string &text,
                                     float complexity,
                                     float char_spacing) {
    return s_font_manager_.GetTextOutlines(font_name, text, complexity,
                                           char_spacing);
}
