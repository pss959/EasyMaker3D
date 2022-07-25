#include "Panes/TextPane.h"

#include <algorithm>

#include "Base/Tuning.h"
#include "Math/Linear.h"
#include "Math/ToString.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "Util/Assert.h"
#include "Util/String.h"

void TextPane::AddFields() {
    AddField(text_.Init("text",                 "."));
    AddField(color_.Init("color",               Color::Black()));
    AddField(font_name_.Init("font_name",       TK::kFontName));
    AddField(font_size_.Init("font_size",       12));
    AddField(resize_text_.Init("resize_text",   true));
    AddField(halignment_.Init("halignment",     HAlignment::kAlignHCenter));
    AddField(valignment_.Init("valignment",     VAlignment::kAlignVCenter));
    AddField(char_spacing_.Init("char_spacing", 1));
    AddField(line_spacing_.Init("line_spacing", 1));
    AddField(padding_.Init("padding",           0));
    AddField(offset_.Init("offset",             Vector2f::Zero()));

    LeafPane::AddFields();
}

bool TextPane::IsValid(std::string &details) {
    if (! LeafPane::IsValid(details))
        return false;
    if (GetText().empty()) {
        details = "Empty text string";
        return false;
    }
    return true;
}

void TextPane::CreationDone() {
    LeafPane::CreationDone();

    if (! IsTemplate()) {
        text_node_ = SG::FindTypedNodeUnderNode<SG::TextNode>(*this, "Text");
        auto &opts = text_node_->GetLayoutOptions();
        ASSERT(opts);
        opts->SetHAlignment(halignment_);
        opts->SetVAlignment(valignment_);
        opts->SetLineSpacing(line_spacing_);
        opts->SetGlyphSpacing(GetGlyphSpacing_(char_spacing_));
        text_node_->SetFontName(font_name_);
        text_node_->SetTextWithColor(text_, color_);
    }
}

void TextPane::SetText(const std::string &text) {
    if (text != GetText()) {
        text_ = text;
        if (text_node_) {
            if (text.empty()) {
                text_node_->SetEnabled(false);
            }
            else {
                text_node_->SetEnabled(true);
                text_node_->SetTextWithColor(text_, color_);
            }
        }
        BaseSizeChanged();
    }
}

void TextPane::SetColor(const Color &color) {
    color_ = color;
    if (text_node_ && ! GetText().empty())
        text_node_->SetTextWithColor(text_, color_);
}

void TextPane::SetFontName(const std::string &font_name) {
    if (font_name != font_name_.GetValue()) {
        font_name_ = font_name;
        if (text_node_)
            text_node_->SetFontName(font_name);
        BaseSizeChanged();
    }
}

void TextPane::SetFontSize(float font_size) {
    if (font_size != font_size_.GetValue()) {
        font_size_ = font_size;
        BaseSizeChanged();
    }
}

void TextPane::SetCharacterSpacing(float spacing) {
    if (spacing != char_spacing_.GetValue()) {
        char_spacing_ = spacing;
        if (text_node_) {
            auto &opts = text_node_->GetLayoutOptions();
            ASSERT(opts);
            opts->SetGlyphSpacing(GetGlyphSpacing_(spacing));
        }
        BaseSizeChanged();
    }
}

void TextPane::SetLayoutSize(const Vector2f &size) {
    LeafPane::SetLayoutSize(size);

    // The size is now known, so fix the transform in the TextNode if it has
    // been set up.
    ASSERT(text_node_);
    if (text_node_->GetTextSize() != Vector2f::Zero())
        UpdateTextTransform_(size);
}

void TextPane::PostSetUpIon() {
    LeafPane::PostSetUpIon();

    // The TextNode does not have a valid size until it can access the Ion font
    // information in SetUpIon(). So now there should be a valid size to set
    // up.
    if (GetLayoutSize() != Vector2f::Zero())
        SetLayoutSize(GetLayoutSize());
}

std::string TextPane::ToString(bool is_brief) const {
    // Add the text size (from the TextNode) and the text string.
    std::string s = text_.GetValue();
    if (s.size() > 16U)
        s = s.substr(0, 13U) + "...";
    return LeafPane::ToString(is_brief) +
        " TS=" + Math::ToString(text_size_, .01f) + " '" + s + "'";
}

Vector2f TextPane::ComputeBaseSize() const {
    ASSERT(text_node_);

    // Compute the unpadded size.
    unpadded_base_size_ = ComputeUnpaddedBaseSize_();

    // Add in the padding in both dimensions and respect the minimum size.
    const Vector2f padding = 2 * padding_ * Vector2f(1, 1);
    return MaxComponents(GetMinSize(), unpadded_base_size_ + padding);
}

void TextPane::SetBaseSize(const Vector2f &new_base_size) {
    LeafPane::SetBaseSize(new_base_size);
    if (text_node_->GetTextSize() != Vector2f::Zero() &&
        GetLayoutSize() != Vector2f::Zero())
        UpdateTextTransform_(GetLayoutSize());
}

bool TextPane::ProcessChange(SG::Change change, const Object &obj) {
    if (! LeafPane::ProcessChange(change, obj)) {
        return false;
    }
    else {
        // This TextPane observes the child SG::TextNode, so if a
        // non-appearance change is detected, there may be a size change.
        if (change != SG::Change::kAppearance)
            BaseSizeChanged();
        return true;
    }
}

Vector2f TextPane::ComputeUnpaddedBaseSize_() const {
    ASSERT(text_node_);

    // Get the size of the text from the TextNode. If it is zero, that means it
    // has not yet had the font set.
    const auto size = text_node_->GetTextSize();
    if (size == Vector2f::Zero())
        return size;

    // The base height is based solely on the font_size, padding, line_spacing,
    // and number of lines of text.
    const std::string &text = text_.GetValue();
    const size_t line_count = 1 + std::count(text.begin(), text.end(), '\n');

    Vector2f base_size(0, 0);

    // Compute the height of the text.
    if (line_count == 1U) {
        base_size[1] = font_size_;
    }
    else {
        // Multi-line text needs some extra work. Use the ratio between the Ion
        // text advance height and the font size to get the spacing between
        // lines of text.
        const float spaced_height =
            text_node_->GetLineSpacingFactor() * line_spacing_ * font_size_;
        base_size[1] = (line_count - 1) * spaced_height + font_size_;
    }

    // Compute the text aspect ratio.
    const float aspect = size[0] / size[1];

    // Use the aspect ratio to compute the width of the text.
    base_size[0] = aspect * base_size[1];

    return base_size;
}

void TextPane::UpdateTextTransform_(const Vector2f &pane_size) const {
    ASSERT(text_node_);

    const Vector2f text_size = text_node_->GetTextSize();
    ASSERT(text_size != Vector2f::Zero());

    // Use that to compute the scale factor to apply to scale the text to fit
    // within the Pane bounds.
    const auto scale = ComputeTextScale_(pane_size, text_size);
    text_node_->SetScale(scale);

    // Compute the translation to apply to postion the text to account for
    // padding and alignment.
    text_node_->SetTranslation(ComputeTextTranslation_(pane_size));

    // Save the full text size.
    text_size_.Set(scale[0] * text_size[0] * pane_size[0],
                   scale[1] * text_size[1] * pane_size[1]);
}

Vector3f TextPane::ComputeTextScale_(const Vector2f &pane_size,
                                     const Vector2f &text_size) const {
    ASSERT(text_node_);

    // This stores the text size after accounting for adjustments to maintain
    // the proper text aspect ratio.
    Vector2f adjusted_text_size = text_size;

    // Fraction of the pane covered by the text.
    Vector2f pane_fraction;

    // If IsHeightResizable() and resize_text_ are both true, text is scaled up
    // in Y from the specified font size.
    const bool resize_text_height = IsHeightResizable() && resize_text_;

    // If the height of the text can be resized, there are two cases, based on
    // the aspect ratios of the text and the unpadded Pane size:
    //
    //   ---------------        ----------------
    //   |  |       |  |        |--------------|
    //   |  | Text  |  |   OR   |     Text     |
    //   |  |       |  |        |--------------|
    //   ---------------        ----------------
    if (resize_text_height) {
        // This has to take padding into account, as the text can fill only the
        // unpadded fraction of the Pane.
        const Vector2f unpadded = pane_size - 2 * padding_ * Vector2f(1, 1);
        pane_fraction = unpadded / pane_size;

        // Get the aspect ratios of the unpadded Pane and the text.
        const float pane_aspect = unpadded[0] / unpadded[1];
        const float text_aspect = text_size[0] / text_size[1];

        if (text_aspect <= pane_aspect)     // First case.
            adjusted_text_size[0] = text_size[1] * pane_aspect;
        else                                // Second case.
            adjusted_text_size[1] = text_size[0] / pane_aspect;
    }

    // If the height of the text cannot be resized, it must be held at the
    // correct font size value. The text height may actually not fill the pane
    // height, so use the unpadded base size relative to the pane size to
    // compute the proper scale.
    else {
        pane_fraction = unpadded_base_size_ / pane_size;
    }

    // Scale so that the text occupies the fraction of the Pane.
    return Vector3f(pane_fraction / adjusted_text_size, 1);
}

Vector3f TextPane::ComputeTextTranslation_(const Vector2f &pane_size) const {
    ASSERT(pane_size[0] > 0 && pane_size[1] > 0);

    // The translation is within (0,0) to (1,1) coordinates, so it is just a
    // fraction of that.

    // Compute the fraction to offset by to adjust for padding.
    const Vector2f pad_frac(padding_ / pane_size[0], padding_ / pane_size[1]);

    const HAlignment halign = halignment_;
    const VAlignment valign = valignment_;

    const float xoffset = .5f - pad_frac[0];
    const float yoffset = .5f - pad_frac[1];

    const Vector2f pos(halign == HAlignment::kAlignLeft ? -xoffset :
                       halign == HAlignment::kAlignRight ? xoffset : 0.f,
                       valign == VAlignment::kAlignBottom ? -yoffset :
                       valign == VAlignment::kAlignTop ? yoffset : 0.f);

    // Always move the text forward a little in case there is a background.
    return Vector3f(pos + offset_, TK::kPaneZOffset);
}

float TextPane::GetGlyphSpacing_(float char_spacing) const {
    // Character spacing is roughly relative to character size, with 1 as the
    // default spacing. Glyph spacing is in pixels, with 0 meaning regular
    // spacing.
    return (char_spacing - 1) * font_size_.GetValue();
}
