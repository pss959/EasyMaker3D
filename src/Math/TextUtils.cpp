#include "Math/TextUtils.h"

// Freetype2 headers.
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "Util/Assert.h"

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

  private:
    typedef std::unordered_map<FilePath, Font> FontMap_;

    FT_Library library_;
    bool       library_initialized_ = false;
    FontMap_   font_map_;

    void InitLibrary_();
    bool CanLoadFace_(FT_Face face);
};

static FontManager_ s_font_manager_;

void FontManager_::InitLibrary_() {
    ASSERT(! library_initialized_);
    if (FT_Init_FreeType(&library_) != FT_Err_Ok)
        ASSERTM(false, "Unable to initialize FreeType2");
    library_initialized_ = true;
}

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
    // XXXX DO SOMETHING!
    return polygons;
}
