#include "SG/TextNode.h"

#include <ion/base/serialize.h>
#include <ion/text/fontmanager.h>
#include <ion/text/layout.h>

#include "Assert.h"
#include "SG/Exception.h"
#include "SG/Tracker.h"
#include "Util/Read.h"

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

void TextNode::SetUpIon(const ContextPtr &context) {
    Node::SetUpIon(context);

    if (! GetIonNode()) {
        if (GetLayoutOptions())
            GetLayoutOptions()->SetUpIon(context);

        // Set up the FontImage.
        font_image_ = GetFontImage_(*context);

        // Create an OutlineBuilder.
        builder_.Reset(new ion::text::OutlineBuilder(
                           font_image_, context->shader_manager,
                           ion::base::AllocatorPtr()));

        // Build the text.
        if (BuildText_()) {
            const ion::gfx::NodePtr &node = builder_->GetNode();
            node->SetLabel(GetName());
            SetIonNode(node);
        }
    }
}

bool TextNode::BuildText_() {
    // Build the Layout.
    ion::text::LayoutOptions opts;
    if (GetLayoutOptions())
        opts = GetLayoutOptions()->GetIonLayoutOptions();

    ASSERT(font_image_);
    const ion::text::Layout layout =
        font_image_->GetFont()->BuildLayout(text_, opts);

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
FontImagePtr TextNode::GetFontImage_(Context &context) const {
    // See if the FontImage was already cached.
    const std::string key = BuildFontImageKey_(font_name_, max_image_size_);
    FontImagePtr image = context.font_manager->GetCachedFontImage(key);
    if (! image) {
        // Locate the font in the font resource directory.
        Util::FilePath font_path =
            Util::FilePath::GetResourcePath("fonts", GetFontName() + ".ttf");
        if (! font_path.Exists())
            throw Exception("Font path '" + font_path.ToString() +
                            "' does not exist");
        // Create the font.
        FontPtr font = context.font_manager->AddFontFromFilePath(
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
            context.font_manager->CacheFontImage(key, image);
        }
        else
            throw Exception("Unable to create font image from path '" +
                            font_path.ToString() + "'");
    }
    ASSERT(image);
    return image;
}

}  // namespace SG
