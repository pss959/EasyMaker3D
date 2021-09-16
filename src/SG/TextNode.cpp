#include "SG/TextNode.h"

#include <ion/base/serialize.h>
#include <ion/text/layout.h>

#include "Assert.h"
#include "SG/Exception.h"
#include "SG/Tracker.h"
#include "Util/Read.h"

using ion::gfxutils::ShaderManagerPtr;
using ion::text::FontImagePtr;
using ion::text::FontManager;
using ion::text::FontPtr;

namespace SG {

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

// Creates a FontImage key string from the font name and image size.
static const std::string BuildFontImageKey_(const std::string &font_name,
                                            size_t image_size) {
    return font_name + '/' + ion::base::ValueToString(image_size);
}

// ----------------------------------------------------------------------------
// TextNode functions.
// ----------------------------------------------------------------------------

void TextNode::AddFields() {
    AddField(text_);
    AddField(font_name_);
    AddField(font_size_);
    AddField(sdf_padding_);
    AddField(max_image_size_);
    AddField(color_);
    AddField(outline_color_);
    AddField(outline_width_);
    AddField(half_smooth_width_);
    AddField(layout_options_);
}

void TextNode::AddIonText(FontManager &font_manager,
                          ShaderManagerPtr &shader_manager) {
    ASSERT(GetIonNode());

    // Set up the FontImage.
    font_image_ = GetFontImage_(font_manager);

    // Create an OutlineBuilder.
    builder_.Reset(new ion::text::OutlineBuilder(font_image_, shader_manager,
                                                 ion::base::AllocatorPtr()));

    // Build the text.
    if (BuildText_()) {
        const ion::gfx::NodePtr &text_node = builder_->GetNode();

        // The OutlineBuilder needs to own its Ion Node, and the base class
        // owns its Ion Node, so just add the builder's node as a child.
        text_node->SetLabel(GetName() + " text");
        GetIonNode()->AddChild(text_node);
    }
}

bool TextNode::BuildText_() {
    // Build the Layout.
    ion::text::LayoutOptions layout_options;
    if (auto &opts = GetLayoutOptions()) {
        layout_options.target_point            = opts->GetTargetPoint();
        layout_options.target_size             = opts->GetTargetSize();
        layout_options.horizontal_alignment    = opts->GetHAlignment();
        layout_options.vertical_alignment      = opts->GetVAlignment();
        layout_options.line_spacing            = opts->GetLineSpacing();
        layout_options.glyph_spacing           = opts->GetGlyphSpacing();
        layout_options.metrics_based_alignment =
            opts->IsUsingMetricsBasedAlignment();
    }

    ASSERT(font_image_);
    const ion::text::Layout layout =
        font_image_->GetFont()->BuildLayout(text_, layout_options);

    if (! builder_->Build(
            layout, ion::gfx::BufferObject::UsageMode::kStaticDraw))
        return false;

    builder_->SetTextColor(color_);
    builder_->SetOutlineColor(outline_color_);
    builder_->SetOutlineWidth(outline_width_);
    builder_->SetHalfSmoothWidth(half_smooth_width_);
    return true;
}

void TextNode::SetText(const std::string &new_text) {
    text_ = new_text;
    if (GetIonNode())
        BuildText_();
}

// Returns a FontImage to represent the given data. Uses a cached version if it
// already exists in the FontManager.
FontImagePtr TextNode::GetFontImage_(FontManager &font_manager) const {
    // See if the FontImage was already cached.
    const std::string key = BuildFontImageKey_(font_name_, max_image_size_);
    FontImagePtr image = font_manager.GetCachedFontImage(key);
    if (! image) {
        // Locate the font in the font resource directory.
        Util::FilePath font_path =
            Util::FilePath::GetResourcePath("fonts", GetFontName() + ".ttf");
        if (! font_path.Exists())
            throw Exception("Font path '" + font_path.ToString() +
                            "' does not exist");
        // Create the font.
        FontPtr font = font_manager.AddFontFromFilePath(
            font_name_, font_path, font_size_, sdf_padding_);
        if (! font)
            throw Exception("Unable to create font from path '" +
                            font_path.ToString() + "'");

        // Create a StaticFontImage for all targeted characters.
        ion::text::GlyphSet glyph_set(ion::base::AllocatorPtr(nullptr));
        font->AddGlyphsForAsciiCharacterRange(1, 127, &glyph_set);
        ion::text::StaticFontImagePtr sfi(
            new ion::text::StaticFontImage(font, max_image_size_,
                                           glyph_set));

        if (sfi->GetImageData().texture &&
            sfi->GetImageData().texture->HasImage(0)) {
            image = sfi;
            font_manager.CacheFontImage(key, image);
        }
        else
            throw Exception("Unable to create font image from path '" +
                            font_path.ToString() + "'");
    }
    ASSERT(image);
    return image;
}

}  // namespace SG
