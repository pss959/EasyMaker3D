#include "SG/TextNode.h"

#include <ion/base/serialize.h>
#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>
#include <ion/text/layout.h>

#include "Base/Tuning.h"
#include "Math/TextUtils.h"
#include "SG/Exception.h"
#include "SG/IonContext.h"
#include "Util/Assert.h"
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

TextNode::~TextNode() {
    if (IsCreationDone() && ! IsTemplate()) {
        if (auto &opts = GetLayoutOptions())
            Unobserve(*opts);
    }
}

void TextNode::AddFields() {
    AddField(text_.Init("text",                           "."));
    AddField(font_name_.Init("font_name",                 TK::k3DFont));
    AddField(font_size_.Init("font_size",                 32U));
    AddField(sdf_padding_.Init("sdf_padding",             8U));
    AddField(max_image_size_.Init("max_image_size",       512U));
    AddField(color_.Init("color",                         Color::White()));
    AddField(outline_color_.Init("outline_color",         Color::Black()));
    AddField(outline_width_.Init("outline_width",         2));
    AddField(half_smooth_width_.Init("half_smooth_width", 3));
    AddField(layout_options_.Init("layout_options"));

    Node::AddFields();
}

bool TextNode::IsValid(std::string &details) {
    if (! Node::IsValid(details))
        return false;
    if (GetText().empty()) {
        details = "Empty text string";
        return false;
    }
    return true;
}

void TextNode::CreationDone() {
    Node::CreationDone();

    if (! IsTemplate()) {
        // Set up notification from LayoutOptions if it is not null.
        if (auto &opts = GetLayoutOptions())
            Observe(*opts);

        // Restrict text rendering to just the lighting pass, since / text
        // should never cause shadows. (The shader is defined by Ion, so there
        // is no easy way of turning it off for the shadow pass.)
        SetPassName("Lighting");
    }
}

void TextNode::SetFontName(const std::string &font_name) {
    if (font_name != font_name_.GetValue()) {
        font_name_ = font_name;
        if (GetIonContext() && ! Util::is_in_unit_test)
            SetUpFont_();
        needs_rebuild_ = true;
    }
}

void TextNode::SetFontSize(unsigned int font_size) {
    if (font_size != font_size_.GetValue()) {
        font_size_ = font_size;
        needs_rebuild_ = true;
    }
}

void TextNode::SetText(const std::string &new_text) {
    ASSERT(! new_text.empty());
    text_ = new_text;
    needs_rebuild_ = true;
}

void TextNode::SetTextColor(const Color &color) {
    color_ = color;
    needs_rebuild_ = true;
}

void TextNode::SetTextWithColor(const std::string &new_text,
                                const Color &color) {
    ASSERT(! new_text.empty());
    text_  = new_text;
    color_ = color;
    needs_rebuild_ = true;
}

void TextNode::SetLayoutOptions(const LayoutOptionsPtr &opts) {
    if (auto &old_opts = GetLayoutOptions())
        Unobserve(*old_opts);
    layout_options_ = opts;
    if (opts)
        Observe(*opts);
}

float TextNode::GetLineSpacingFactor() const {
    ASSERT(font_image_);

    // Return the ratio of the line advance height to the font size.
    const auto &font = font_image_->GetFont();
    return font->GetFontMetrics().line_advance_height / font->GetSizeInPixels();
}

const Bounds & TextNode::GetTextBounds() {
    if (needs_rebuild_ && font_image_)
        BuildText_();
    return text_bounds_;
}

const Vector2f & TextNode::GetTextSize() {
    if (needs_rebuild_ && font_image_)
        BuildText_();
    return text_size_;
}

ion::gfx::NodePtr TextNode::SetUpIon(
    const IonContextPtr &ion_context,
    const std::vector<ion::gfx::ShaderProgramPtr> &programs) {
    // Let the base class set up the Ion Node.
    auto ion_node = Node::SetUpIon(ion_context, programs);
    ASSERT(ion_node);

    if (! Util::is_in_unit_test) {
        SetUpFont_();

        // Build the text.
        if (BuildText_()) {
            const ion::gfx::NodePtr &text_node = builder_->GetNode();

            // The OutlineBuilder needs to own its Ion Node, and the base class
            // owns its Ion Node, so just add the builder's node as a child.
            text_node->SetLabel(GetName() + " text");
            ion_node->AddChild(text_node);
        }
        else {
            throw Exception("Unable to build Ion text for " + GetDesc() +
                            " with text '" + GetText() + "'");
        }
    }

    return ion_node;
}

void TextNode::UpdateForRenderPass(const std::string &pass_name) {
    Node::UpdateForRenderPass(pass_name);
    if (needs_rebuild_)
        BuildText_();
}

void TextNode::SetWorldScaleAndRotation(const Matrix4f &owm, float base_scale,
                                        const Rotationf &current_rotation) {
    using ion::math::Length;
    using ion::math::Normalized;

    // Compute a relative size in world coordinates and scale.
    const float size = Length(owm * Normalized(Vector3f(1, 1, 1)));
    SetUniformScale(base_scale / size);

    // Rotate to the world Z axis.
    const Rotationf rot = Rotationf::RotateInto(owm * Vector3f::AxisZ(),
                                                Vector3f::AxisZ());
    SetRotation(-current_rotation * rot);
}

Bounds TextNode::UpdateBounds() const {
    return text_bounds_;
}

bool TextNode::ProcessChange(Change change, const Object &obj) {
    if (! Node::ProcessChange(change, obj)) {
        return false;
    }
    else {
        needs_rebuild_ = true;
        return true;
    }
}

void TextNode::SetUpFont_() {
    // This works only after SetUpIon() is called.
    const auto &ion_context = GetIonContext();
    ASSERT(ion_context);
    ASSERT(GetIonNode());

    // Set up the FontImage.
    font_image_ = GetFontImage_(*ion_context->GetFontManager());

    // Create or update the OutlineBuilder.
    if (! builder_) {
        builder_.Reset(new ion::text::OutlineBuilder(
                           font_image_, ion_context->GetShaderManager(),
                           ion::base::AllocatorPtr()));
    }
    else {
        builder_->SetFontImage(font_image_);
    }
}

bool TextNode::BuildText_() {
    ASSERT(needs_rebuild_);

    // Build the Layout.
    ion::text::LayoutOptions ion_opts;
    if (auto &opts = GetLayoutOptions()) {
        ion_opts.horizontal_alignment = opts->GetHAlignment();
        ion_opts.vertical_alignment   = opts->GetVAlignment();
        ion_opts.line_spacing         = opts->GetLineSpacing();
        ion_opts.glyph_spacing        = opts->GetGlyphSpacing();
        ion_opts.metrics_based_alignment =
            opts->IsUsingMetricsBasedAlignment();
    }

    ASSERT(font_image_);
    const ion::text::Layout layout =
        font_image_->GetFont()->BuildLayout(text_, ion_opts);

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
        ProcessChange(Change::kBounds, *this);
    }
    text_size_ = layout.GetSize();

    needs_rebuild_ = false;

    return true;
}

FontImagePtr TextNode::GetFontImage_(FontManager &font_manager) const {
    // See if the FontImage was already cached.
    const std::string &font_name = GetFontName();
    const std::string key = BuildFontImageKey_(font_name, max_image_size_);
    FontImagePtr image = font_manager.GetCachedFontImage(key);
    if (! image) {
        // Create the font.
        const FilePath font_path = GetFontPath(font_name);
        if (! font_path)
            throw Exception("Font '" + font_name + "' does not exist");
        FontPtr font = font_manager.AddFontFromFilePath(
            font_name, font_path.ToString(), font_size_, sdf_padding_);
        if (! font)
            throw Exception("Unable to create font from path '" +
                            font_path.ToString() + "'");

        // Create a StaticFontImage for all targeted characters.
        ion::text::GlyphSet glyph_set(ion::base::AllocatorPtr(nullptr));
        font->AddGlyphsForAsciiCharacterRange(1, 127, &glyph_set);
        // Add the degree sign:
        glyph_set.insert(font->GetDefaultGlyphForChar(0xb0));
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
