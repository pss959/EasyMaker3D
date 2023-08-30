#include "Util/FontSystem.h"

// Freetype2 headers.
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <algorithm>
#include <unordered_map>

#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/KLog.h"

// ----------------------------------------------------------------------------
// FontSystem::Impl_ class.
// ----------------------------------------------------------------------------

class FontSystem::Impl_ {
  public:
    StrVec GetAvailableFontNames();
    bool IsValidFontName(const Str &font_name);
    bool IsValidStringForFont(const Str &font_name, const Str &str,
                              Str &reason);
    FilePath GetFontPath(const Str &font_name);
    void GetTextOutlines(const Str &font_name, const Str &text,
                         float char_spacing, const OutlineFuncs &funcs);

  private:
    typedef std::unordered_map<Str, FilePath> PathMap_;
    typedef std::unordered_map<Str, FT_Face>  FaceMap_;

    bool     is_initialized_ = false;
    PathMap_ path_map_;  ///< Maps font names to FilePath instances.
    FaceMap_ face_map_;  ///< Maps font names to FT_Face instances.

    /// Loads all fonts found in the fonts directory. This must be called
    /// before any other functions. It is safe to call multiple times.
    void Init_();

    bool CanLoadFace_(FT_Face face);

    /// Calls the OutlineFuncs for the outlines representing the glyph for the
    /// given character in the given font to the vector and returns the X
    /// advance amount for the glyph. The starting X value for the glyph is
    /// provided.
    float AddGlyphOutlines_(FT_Face face, char c, float x_start,
                            const OutlineFuncs &funcs);

    /// Converts from Q26.6 fixed format to a float.
    static float FromQ26_6_(FT_Pos n) { return n / 64.f; }

    /// Converts from Q16.16 fixed format to a float.
    static float FromQ16_16_(FT_Pos n) { return n / 65536.f; }
};

StrVec FontSystem::Impl_::GetAvailableFontNames() {
    Init_();
    StrVec names = Util::GetKeys(face_map_);
    std::sort(names.begin(), names.end());
    return names;
}

bool FontSystem::Impl_::IsValidFontName(const Str &font_name) {
    Init_();
    return face_map_.contains(font_name);
}

bool FontSystem::Impl_::IsValidStringForFont(const Str &font_name,
                                             const Str &str, Str &reason) {
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
    Str bad_chars;
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

FilePath FontSystem::Impl_::GetFontPath(const Str &font_name) {
    Init_();
    const auto it = path_map_.find(font_name);
    return it == path_map_.end() ? FilePath() : it->second;
}

void FontSystem::Impl_::GetTextOutlines(const Str &font_name, const Str &text,
                                        float char_spacing,
                                        const OutlineFuncs &funcs) {
    Init_();
    const auto it = face_map_.find(font_name);
    if (it != face_map_.end()) {
        const auto &face = it->second;
        // Process each character in the text.
        float x = 0;
        for (const char c: text) {
            const float advance =
                AddGlyphOutlines_(face, c, x, funcs);
            x += char_spacing * advance;
        }
    }
}

void FontSystem::Impl_::Init_() {
    if (is_initialized_)
        return;

    // Initialize the Freetype2 library.
    FT_Library lib;
    if (FT_Init_FreeType(&lib) != FT_Err_Ok) {
        // LCOV_EXCL_START
        ASSERTM(false, "Unable to initialize FreeType2");
        return;
        // LCOV_EXCL_STOP
    }

    // Access all font files.
    const FilePath dir_path = FilePath::GetResourcePath("fonts", FilePath());
    StrVec subdirs;
    StrVec files;
    dir_path.GetContents(subdirs, files, ".ttf", false);

    for (const auto &f: files) {
        const FilePath path = FilePath::Join(dir_path, f);

        // Create a new font face and make sure it can be loaded. If so, add it
        // to the maps.
        FT_Face face;
        if (FT_New_Face(lib, path.ToString().c_str(), 0, &face) == FT_Err_Ok &&
            CanLoadFace_(face)) {
            const Str name = Str(face->family_name) + "-" + face->style_name;
            KLOG('z', "Loaded font '" << name << " from path '"
                 << path.ToString() << "'");
            ASSERTM(! face_map_.contains(name), name);
            path_map_[name] = path;
            face_map_[name] = face;
        }
    }

    is_initialized_ = true;
}

bool FontSystem::Impl_::CanLoadFace_(FT_Face face) {
    // Load the glyph for 'A'.
    FT_UInt glyph_index = FT_Get_Char_Index(face, 'A');
    if (FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_SCALE) != FT_Err_Ok)
        return false;  // LCOV_EXCL_LINE

    FT_Glyph glyph;
    if (FT_Get_Glyph(face->glyph, &glyph) != FT_Err_Ok)
        return false;  // LCOV_EXCL_LINE

    if (glyph->format != FT_GLYPH_FORMAT_OUTLINE)
        return false;  // LCOV_EXCL_LINE

    return true;
}

float FontSystem::Impl_::AddGlyphOutlines_(FT_Face face, char c, float x_start,
                                           const OutlineFuncs &funcs) {
    // Get the glyph.
    FT_UInt glyph_index = FT_Get_Char_Index(face, c);
    FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_SCALE);
    FT_Glyph glyph;
    FT_Get_Glyph(face->glyph, &glyph);

    // Get the glyph outline.
    const FT_OutlineGlyph outline_glyph =
        reinterpret_cast<FT_OutlineGlyph>(glyph);
    FT_Outline *outline = &outline_glyph->outline;

    funcs.begin_outline_func(outline->n_contours);

    int cur_point = 0;
    for (int i = 0; i < outline->n_contours; ++i) {
        // Begin the contour.
        const int contour_end = outline->contours[i];
        const int point_count = contour_end - cur_point + 1;
        funcs.begin_border_func(point_count);

        for (; cur_point <= contour_end; ++cur_point) {
            const FT_Vector &pt     = outline->points[cur_point];
            const unsigned int tags = outline->tags[cur_point];
            const bool is_on_curve  = FT_CURVE_TAG(tags) == FT_Curve_Tag_On;

            // Point coordinates are in Q26.6 format.
            funcs.add_point_func(x_start + FromQ26_6_(pt.x), FromQ26_6_(pt.y),
                                 is_on_curve);
        }
    }

    // Advance is in Q16.16 fixed format.
    return FromQ16_16_(glyph->advance.x);
}

// ----------------------------------------------------------------------------
// FontSystem functions.
// ----------------------------------------------------------------------------

FontSystem::FontSystem() : impl_(new Impl_) {}

FontSystem::~FontSystem() {}

StrVec FontSystem::GetAvailableFontNames() const {
    return impl_->GetAvailableFontNames();
}

bool FontSystem::IsValidFontName(const Str &font_name) const {
    return impl_->IsValidFontName(font_name);
}

bool FontSystem::IsValidStringForFont(const Str &font_name, const Str &str,
                                      Str &reason) const {
    return impl_->IsValidStringForFont(font_name, str, reason);
}

FilePath FontSystem::GetFontPath(const Str &font_name) const {
    return impl_->GetFontPath(font_name);
}

void FontSystem::GetTextOutlines(const Str &font_name, const Str &text,
                                 float char_spacing,
                                 const OutlineFuncs &funcs) const {
    ASSERT(funcs.begin_outline_func);
    ASSERT(funcs.begin_border_func);
    ASSERT(funcs.add_point_func);

    impl_->GetTextOutlines(font_name, text, char_spacing, funcs);
}
