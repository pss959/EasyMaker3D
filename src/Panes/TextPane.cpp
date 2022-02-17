#include "Panes/TextPane.h"

#include <algorithm>

#include "Math/Linear.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "Util/Assert.h"
#include "Util/String.h"

void TextPane::AddFields() {
    AddField(text_);
    AddField(color_);
    AddField(font_name_);
    AddField(font_size_);
    AddField(halignment_);
    AddField(valignment_);
    AddField(line_spacing_);
    AddField(padding_);
    Pane::AddFields();
}

bool TextPane::IsValid(std::string &details) {
    if (! Pane::IsValid(details))
        return false;
    if (GetText().empty()) {
        details = "Empty text string";
        return false;
    }
    return true;
}

void TextPane::CreationDone() {
    Pane::CreationDone();

    if (! IsTemplate()) {
        text_node_ = SG::FindTypedNodeUnderNode<SG::TextNode>(*this, "Text");
        auto &opts = text_node_->GetLayoutOptions();
        ASSERT(opts);
        opts->SetHAlignment(halignment_);
        opts->SetVAlignment(valignment_);
        opts->SetLineSpacing(line_spacing_);
        text_node_->SetFontName(font_name_);
        text_node_->SetTextWithColor(text_, color_);
    }
}

void TextPane::SetText(const std::string &text) {
    ASSERT(! text.empty());
    if (text != GetText()) {
        text_ = text;
        if (text_node_) {
            text_node_->SetTextWithColor(text_, color_);
            // The new text may change the size.
            SizeChanged(*this);
        }
    }
}

void TextPane::SetColor(const Color &color) {
    color_ = color;
    if (text_node_)
        text_node_->SetTextWithColor(text_, color_);
}

void TextPane::SetFontName(const std::string &font_name) {
    if (font_name != font_name_.GetValue()) {
        font_name_ = font_name;
        if (text_node_)
            text_node_->SetFontName(font_name);
    }
}

void TextPane::SetSize(const Vector2f &size) {
    Pane::SetSize(size);

    // The size is now known, so fix the transform in the TextNode.
    ASSERT(text_node_);
    UpdateTextTransform_(size);
}

std::string TextPane::ToString() const {
    // Add the text size (from the TextNode) and the text string.
    std::string s = text_.GetValue();
    if (s.size() > 16U)
        s = s.substr(0, 13U) + "...";
    return Pane::ToString() +
        " TS=" + Util::ToString(text_size_, .01f) + " '" + s + "'";
}

Vector2f TextPane::ComputeBaseSize() const {
    ASSERT(text_node_);

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

    // Get the size of the text from the TextNode and compute its aspect ratio.
    const auto size = text_node_->GetTextSize();
    const float aspect = size[0] / size[1];

    // Use the aspect ratio to compute the width of the text.
    base_size[0] = aspect * base_size[1];

    // Add the padding in both dimensions..
    base_size += 2 * padding_ * Vector2f(1, 1);

    // Respect the minimum size.
    return MaxComponents(Pane::GetMinSize(), base_size);
}

bool TextPane::ProcessChange(SG::Change change, const Object &obj) {
    if (! Pane::ProcessChange(change, obj)) {
        return false;
    }
    else {
        // This TextPane observes the child SG::TextNode, so if a
        // non-appearance change is detected, there may be a size change.
        if (change != SG::Change::kAppearance)
            SizeChanged(*this);
        return true;
    }
}

void TextPane::UpdateTextTransform_(const Vector2f &pane_size) {
    ASSERT(text_node_);
    ASSERT(IsSizeKnown());

    // Get the size of the (unscaled) bounds of the text.
    const Vector2f text_size = text_node_->GetTextSize();

    // Use that to compute the scale factor to apply to scale the text to fit
    // within the Pane bounds.
    text_node_->SetScale(ComputeTextScale_(pane_size, text_size));

    // Compute the translation to apply to postion the text to account for
    // padding and alignment.
    text_node_->SetTranslation(ComputeTextTranslation_(pane_size));

    // Save the full text size.
    const auto scale = text_node_->GetScale();
    ASSERT(text_size != Vector2f::Zero());
    text_size_.Set(scale[0] * text_size[0] * pane_size[0],
                   scale[1] * text_size[1] * pane_size[1]);
}

Vector3f TextPane::ComputeTextScale_(const Vector2f &pane_size,
                                     const Vector2f &text_size) {
    ASSERT(text_node_);

    // This has to take padding into account, as the text can fill only the
    // unpadded fraction of the Pane.
    const float padding = 2 * padding_;
    const Vector2f unpadded_pane_size(pane_size[0] - padding,
                                      pane_size[1] - padding);
    const Vector2f unpadded_fraction(unpadded_pane_size[0] / pane_size[0],
                                     unpadded_pane_size[1] / pane_size[1]);

    // Get the aspect ratios of the unpadded Pane and the text.
    const float pane_aspect = unpadded_pane_size[0] / unpadded_pane_size[1];
    const float text_aspect = text_size[0] / text_size[1];

    Vector3f scale(1, 1, 1);

    // If the height of the text can be resized, there are two cases, based on
    // the aspect ratios of the text and the unpadded Pane size:
    //
    //   ---------------        ----------------
    //   |  |       |  |        |--------------|
    //   |  | Text  |  |   OR   |     Text     |
    //   |  |       |  |        |--------------|
    //   ---------------        ----------------
    if (IsHeightResizable()) {
        if (text_aspect <= pane_aspect) {     // First case.
            scale[0] = unpadded_fraction[0] / (text_size[1] * pane_aspect);
            scale[1] = unpadded_fraction[1] / text_size[1];
        }
        else {                                // Second case.
            scale[0] = unpadded_fraction[0] / text_size[0];
            scale[1] = unpadded_fraction[1] / (text_size[0] / pane_aspect);
        }
    }

    // If the height of the text cannot be resized, it must be held at the
    // correct font size value (which fills the unpadded height). The width is
    // computed so that the aspect ratio of the text stays correct.
    //
    //     ---------------
    //     |  | Text  |  |
    //     ---------------
    else {
        scale[0] = unpadded_fraction[0] / (text_size[1] * pane_aspect);
        scale[1] = unpadded_fraction[1] / text_size[1];
    }

    return scale;
}

Vector3f TextPane::ComputeTextTranslation_(const Vector2f &pane_size) {
    // The translattion is within (0,0) to (1,1) coordinates, so it is just a
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
    return Vector3f(pos + offset_, .1f);
}
