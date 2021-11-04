#include "Panes/TextPane.h"

#include "Assert.h"
#include "SG/Search.h"
#include "SG/TextNode.h"

void TextPane::AddFields() {
    AddField(text_);
    Pane::AddFields();
}

void TextPane::SetText(const std::string &text) {
    text_ = text;
    if (! IsTemplate() && text_node_)
        text_node_->SetText(text_);
}

void TextPane::PreSetUpIon() {
    Pane::PreSetUpIon();
    if (! text_node_ && ! IsTemplate()) {
        text_node_ = SG::FindTypedNodeUnderNode<SG::TextNode>(*this, "Text");
        SetSize(GetSize());
    }
    SetText(text_);
}

void TextPane::PostSetUpIon() {
    // Save the computed text size.
    if (! IsTemplate()) {
        const auto size = text_node_->GetTextBounds().GetSize();
        text_size_.Set(size[0], size[1]);
        SetMinSize(ComputeMinSize());
    }
}

void TextPane::SetSize(const Vector2f &size) {
    Pane::SetSize(size);
    if (text_node_ && text_size_[0] > 0) {
        auto &opts = text_node_->GetLayoutOptions();
        const Vector2f fixed_size = GetFixedSize_();
        opts->SetTargetSize(
            Vector2f(IsWidthResizable()  ? 1 : fixed_size[0] / size[0],
                     IsHeightResizable() ? 1 : fixed_size[1] / size[1]));
        if (! IsWidthResizable() || ! IsHeightResizable())
            SetMinSize(fixed_size);
    }
}

Vector2f TextPane::ComputeMinSize() const {
    Vector2f min_size = Pane::ComputeMinSize();
    if (text_size_[0] > 0 && text_size_[1] > 0) {
        const Vector2f fixed_size = GetFixedSize_();
        if (! IsWidthResizable())
            min_size[0] = std::max(min_size[0], fixed_size[0]);
        if (! IsHeightResizable())
            min_size[1] = std::max(min_size[1], fixed_size[1]);
    }
    return min_size;
}

Vector2f TextPane::GetFixedSize_() const {
    const Vector2f base_size = GetBaseSize();
    return Vector2f(base_size[0] * text_size_[0],
                    base_size[1] * text_size_[1]);
}
