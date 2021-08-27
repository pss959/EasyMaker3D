#include "SG/TextNode.h"

#include <ion/base/serialize.h>
#include <ion/text/fontmanager.h>
#include <ion/text/layout.h>

#include "Assert.h"
#include "SG/Exception.h"
#include "SG/LayoutOptions.h"
#include "SG/SpecBuilder.h"
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

void TextNode::SetUpIon(IonContext &context) {
    if (! GetIonNode()) {
        if (layout_options_)
            layout_options_->SetUpIon(context);

        // Set up the FontImage.
        font_image_ = GetFontImage_(context);

        // Create an OutlineBuilder.
        builder_.Reset(new ion::text::OutlineBuilder(
                           font_image_, context.shader_manager,
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
    if (layout_options_)
        opts = layout_options_->GetIonLayoutOptions();

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

Parser::ObjectSpec TextNode::GetObjectSpec() {
    SG::SpecBuilder<TextNode> builder;
    builder.AddString("text",                  &TextNode::text_);
    builder.AddString("font_name",             &TextNode::font_name_);
    builder.AddUInt("font_size",               &TextNode::font_size_);
    builder.AddUInt("sdf_padding",             &TextNode::sdf_padding_);
    builder.AddUInt("max_image_size",          &TextNode::max_image_size_);
    builder.AddVector4f("color",               &TextNode::color_);
    builder.AddVector4f("outline_color",       &TextNode::outline_color_);
    builder.AddFloat("outline_width",          &TextNode::outline_width_);
    builder.AddFloat("half_smooth_width",      &TextNode::half_smooth_width_);
    builder.AddObject<LayoutOptions>("layout", &TextNode::layout_options_);
    return Parser::ObjectSpec{
        "TextNode", false, []{ return new TextNode; }, builder.GetSpecs() };
}

// Returns a FontImage to represent the given data. Uses a cached version if it
// already exists in the FontManager.
FontImagePtr TextNode::GetFontImage_(IonContext &context) const {
    // See if the FontImage was already cached.
    const std::string key = BuildFontImageKey_(font_name_, max_image_size_);
    FontImagePtr image = context.font_manager->GetCachedFontImage(key);
    if (! image) {
        // Locate the font in the font resource directory.
        Util::FilePath font_path =
            Util::FilePath::GetResourcePath("fonts", font_name_ + ".ttf");
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
