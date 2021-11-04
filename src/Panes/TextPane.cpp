#include "Panes/TextPane.h"

#include "Assert.h"
#include "SG/Search.h"
#include "SG/TextNode.h"

#include "Math/Linear.h" // XXXX

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

        // The main goal here is to set the target_size in the LayoutOptions. A
        // target_size of 1 in either dimension means that the text fills its
        // rectangle. A smaller value uses just a fraction of the rectangle.
        // The other constraint is that the text's aspect ratio should not
        // change.
        //
        // There are two cases, based on the aspect ratios of the text and the
        // given size (text is smaller rectangle):
        //
        //   ---------------        ---------------
        //   |  |       |  |        |--------------|
        //   |  |       |  |   OR   |              |
        //   |  |       |  |        |--------------|
        //   ---------------        ----------------
        //
        // The resulting real width and height of the TextPane:
        //      w = target_size[0] * size[0]
        //      h = target_size[1] * size[1]
        //
        // Keeping the text's aspect ratio constant means that:
        //      w / h = text_aspect
        //
        //  Let T = target_size, S = size, A = text_aspect
        //     (T0 * S0) / (T1 * S1) = A
        //
        //  If T1 is 1 (first picture), then
        //     (T0 * S0) / S1 = A
        //            T0 * S0 = A * S1
        //                 T0 = A * S1 / T0
        //
        //  If T0 is 1 (second picture), then
        //     S0 / (T1 * S1) = A
        //                 S0 = A * T1 * S1
        //                 T1 = S0 / (A * S1)

        const float text_aspect = text_size_[0] / text_size_[1];
        const float size_aspect = size[0] / size[1];
        if (text_aspect <= size_aspect) {  // First picture.
            const float t0 = text_aspect * size[1] / size[0];
            opts->SetTargetSize(Vector2f(t0, 1));
        }
        else {                             // Second picture.
            const float t1 = size[0] / (text_aspect * size[1]);
            opts->SetTargetSize(Vector2f(1, t1));
        }
    }
}

Vector2f TextPane::ComputeMinSize() const {
    return MaxComponents(Pane::ComputeMinSize(), GetFixedSize_());
}

Vector2f TextPane::GetFixedSize_() const {
    const Vector2f base_size = GetBaseSize();
    return Vector2f(base_size[0] * text_size_[0],
                    base_size[1] * text_size_[1]);
}
