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
    Node::AddFields();
}

bool TextNode::IsValid(std::string &details) {
    if (! Node::IsValid(details))
        return false;

    // Set up notification from LayoutOptions. If this TextNode is a clone,
    // skip the LayoutOptions if it is also a clone, since it would have
    // already been set up in CopyContentsFrom().
    if (auto &layout = GetLayoutOptions()) {
        if (GetObjectType() != Parser::Object::ObjType::kClone &&
            layout->GetObjectType() != Parser::Object::ObjType::kClone)
            Observe(*layout);
    }

    return true;
}

void TextNode::SetText(const std::string &new_text) {
    text_ = new_text;
    needs_rebuild_ = true;
}

void TextNode::SetTextWithColor(const std::string &new_text,
                                const Color &color) {
    text_  = new_text;
    color_ = color;
    needs_rebuild_ = true;
}

void TextNode::SetLayoutOptions(const LayoutOptionsPtr &layout) {
    if (GetLayoutOptions())
        Unobserve(*GetLayoutOptions());
    layout_options_ = layout;
    if (layout)
        Observe(*layout);
}

ion::gfx::NodePtr TextNode::SetUpIon(
    const IonContextPtr &ion_context,
    const std::vector<ion::gfx::ShaderProgramPtr> &programs) {
    // Let the base class set up the Ion Node.
    auto ion_node = Node::SetUpIon(ion_context, programs);
    ASSERT(ion_node);

    // Set up the FontImage.
    font_image_ = GetFontImage_(*ion_context->GetFontManager());

    // Create an OutlineBuilder.
    builder_.Reset(new ion::text::OutlineBuilder(
                       font_image_, ion_context->GetShaderManager(),
                       ion::base::AllocatorPtr()));

    // Build the text.
    if (BuildText_()) {
        const ion::gfx::NodePtr &text_node = builder_->GetNode();

        // The OutlineBuilder needs to own its Ion Node, and the base class
        // owns its Ion Node, so just add the builder's node as a child.
        text_node->SetLabel(GetName() + " text");
        ion_node->AddChild(text_node);
    }
    else {
        throw Exception("Unable to build Ion text for " + GetDesc());
    }

    return ion_node;
}

void TextNode::UpdateForRenderPass(const std::string &pass_name) {
    Node::UpdateForRenderPass(pass_name);
    if (needs_rebuild_)
        BuildText_();
}

void TextNode::CopyContentsFrom(const Parser::Object &from, bool is_deep) {
    Node::CopyContentsFrom(from, is_deep);

    // Add observer to LayoutOptions.
    if (auto &layout = GetLayoutOptions())
        Observe(*layout);
}

Bounds TextNode::UpdateBounds() const {
    return text_bounds_;
}

void TextNode::ProcessChange(Change change) {
    Node::ProcessChange(change);
    needs_rebuild_ = true;
}

bool TextNode::BuildText_() {
    ASSERT(needs_rebuild_);

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

    // Save the text bounds. If they changed, notify observers.
    const Bounds new_bounds = builder_->GetExtents();
    if (new_bounds != text_bounds_) {
        text_bounds_ = new_bounds;
        ProcessChange(Change::kBounds);
    }

    needs_rebuild_ = false;

    return true;
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
        // Add the degree sign:
        glyph_set.insert(font->GetDefaultGlyphForChar(0xb0));  // Degree sign.
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
