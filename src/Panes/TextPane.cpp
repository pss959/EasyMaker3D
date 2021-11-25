#include "Panes/TextPane.h"

#include <algorithm>

#include "Assert.h"
#include "Defaults.h"
#include "Math/Linear.h"
#include "SG/Search.h"
#include "SG/TextNode.h"

void TextPane::AddFields() {
    AddField(text_);
    AddField(font_name_);
    AddField(halignment_);
    AddField(valignment_);
    AddField(padding_);
    Pane::AddFields();
}

void TextPane::SetText(const std::string &text) {
    text_ = text;
    if (text_node_) {
        text_node_->SetText(text_);
        if (GetSize() != Vector2f::Zero())
            UpdateTextTransform_();
    }
}

void TextPane::SetSize(const Vector2f &size) {
    Pane::SetSize(size);

    if (text_node_)
        UpdateTextTransform_();
}

void TextPane::PreSetUpIon() {
    Pane::PreSetUpIon();
    if (! IsTemplate() && ! text_node_)
        text_node_ = SG::FindTypedNodeUnderNode<SG::TextNode>(*this, "Text");
    if (text_node_) {
        auto &opts = text_node_->GetLayoutOptions();
        ASSERT(opts);
        opts->SetHAlignment(halignment_);
        opts->SetVAlignment(valignment_);
        text_node_->SetFontName(font_name_);
        text_node_->SetText(text_);
    }
}

void TextPane::PostSetUpIon() {
    Pane::PostSetUpIon();

    // The text has been built, so update the text size and placement if the
    // pane size is known.
    if (text_node_ && GetSize() != Vector2f::Zero())
        UpdateTextTransform_();
}

std::string TextPane::ToString() const {
    return Pane::ToString() + " '" + text_.GetValue() + "'";
}

Vector2f TextPane::ComputeMinSize() const {
    // Get the size of the text from the TextNode and compute its aspect ratio.
    if (text_node_) {
        const auto size = text_node_->GetTextBounds().GetSize();
        const float aspect = size[0] / size[1];

        const float min_height = Defaults::kMinimumPaneTextHeight;
        const Vector2f min_size(2 * padding_ + min_height * aspect,
                                2 * padding_ + min_height);

        return MaxComponents(Pane::ComputeMinSize(), min_size);
    }
    else {
        return Pane::ComputeMinSize();
    }
}

void TextPane::ProcessChange(SG::Change change) {
    Pane::ProcessChange(change);

    // The only thing this TextPane observes is the child SG::TextNode. If it
    // changes the minimum size, notify.
    if (change != SG::Change::kAppearance && text_node_) {
        const auto cur_min_size = GetMinSize();
        const auto new_min_size = ComputeMinSize();
        if (new_min_size != cur_min_size) {
            SetMinSize(new_min_size);
            ProcessSizeChange();
        }
    }
}

void TextPane::UpdateTextTransform_() {
    ASSERT(text_node_);

    // Update the minimum size.
    const auto min_size = ComputeMinSize();
    SetMinSize(min_size);

    // Compute the scale and translation to apply to the text.
    text_node_->SetScale(ComputeTextScale_());
    text_node_->SetTranslation(ComputeTextTranslation_());

    // Save the full text size.
    const auto scale     = text_node_->GetScale();
    const auto pane_size = GetSize();
    const auto text_size = text_node_->GetTextBounds().GetSize();
    ASSERT(text_size != Vector3f::Zero());
    text_size_.Set(scale[0] * text_size[0] * pane_size[0],
                   scale[1] * text_size[1] * pane_size[1]);
}

Vector3f TextPane::ComputeTextScale_() {
    ASSERT(text_node_);

    // Since the text is scaled with the Pane, the text dimensions after local
    // scaling should be at most 1. Because of padding, the maximum value in a
    // dimension is actually the fraction of that dimension that is not
    // padding. This results in a nonuniform scale to maintain the proper
    // aspect ratio of the original text.
    //
    // There are two cases, based on the aspect ratios of the text and the pane
    // size (with padding removed).
    //
    //   ---------------        ----------------
    //   |  |       |  |        |--------------|
    //   |  | Text  |  |   OR   |     Text     |
    //   |  |       |  |        |--------------|
    //   ---------------        ----------------

    const Vector2f pane_size = GetSize();
    const Vector2f unpadded_pane_size(pane_size[0] - 2 * padding_,
                                      pane_size[1] - 2 * padding_);
    const Vector2f unpadded_fraction(unpadded_pane_size[0] / pane_size[0],
                                     unpadded_pane_size[1] / pane_size[1]);

    const Vector3f text_size = text_node_->GetTextBounds().GetSize();

    const float pane_aspect = unpadded_pane_size[0] / unpadded_pane_size[1];
    const float text_aspect = text_size[0] / text_size[1];

    Vector3f scale(1, 1, 1);
    if (text_aspect <= pane_aspect) {  // First case.
        scale[0] = unpadded_fraction[0] / (text_size[1] * pane_aspect);
        scale[1] = unpadded_fraction[1];
    }
    else {                             // Second case.
        scale[0] = unpadded_fraction[0] / text_size[0];
        scale[1] = (pane_aspect * unpadded_fraction[1]) / text_aspect;
    }
    return scale;
}

Vector3f TextPane::ComputeTextTranslation_() {
    // Compute the fraction to offset by to adjust for padding.
    const Vector2f pane_size = GetSize();
    const Vector2f pad_frac(padding_ / pane_size[0], padding_ / pane_size[1]);

    const HAlignment halign = halignment_;
    const VAlignment valign = valignment_;

    const float xoffset = .5f - pad_frac[0];
    const float yoffset = .5f - pad_frac[1];
    const float x = halign == HAlignment::kAlignLeft ? -xoffset :
        halign == HAlignment::kAlignRight ? xoffset : 0.f;
    const float y = valign == VAlignment::kAlignBottom ? -yoffset :
        valign == VAlignment::kAlignTop ? yoffset : 0.f;

    return Vector3f(x, y, 0);
}
