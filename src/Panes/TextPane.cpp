#include "Panes/TextPane.h"

#include "Assert.h"
#include "Math/Linear.h"
#include "SG/Search.h"
#include "SG/TextNode.h"

void TextPane::AddFields() {
    AddField(text_);
    AddField(font_name_);
    AddField(halignment_);
    AddField(valignment_);
    Pane::AddFields();
}

void TextPane::SetText(const std::string &text) {
    text_ = text;
    if (text_node_) {
        text_node_->SetText(text_);
        UpdateTextSize_();
    }
}

void TextPane::PreSetUpIon() {
    Pane::PreSetUpIon();
    if (! IsTemplate() && ! text_node_)
        text_node_ = SG::FindTypedNodeUnderNode<SG::TextNode>(*this, "Text");
    if (text_node_)
        text_node_->SetText(text_);
}

void TextPane::PostSetUpIon() {
    Pane::PostSetUpIon();
    if (text_node_) {
        text_node_->SetFontName(font_name_);
        auto &opts = text_node_->GetLayoutOptions();
        ASSERT(opts);
        opts->SetHAlignment(halignment_);
        opts->SetVAlignment(valignment_);
        opts->SetTargetSize(Vector2f(0, 1));

        // Make sure the size is correct now that the text has been built.
        SetSize(GetSize());
        UpdateTextSize_();
    }
}

void TextPane::SetSize(const Vector2f &size) {
    Pane::SetSize(size);

    if (text_node_ && text_size_[0] > 0) {
        SetTargetSize_(size);
        SetTargetPoint_();
    }
}

Vector2f TextPane::ComputeMinSize() const {
    return MaxComponents(Pane::ComputeMinSize(), GetFixedSize_());
}

void TextPane::ProcessChange(SG::Change change) {
    Pane::ProcessChange(change);

    // The only thing this TextPane observes is the child SG::TextNode. If it
    // chanages anything that could affect the bounds, consider it a size
    // change to this TextPane.
    if (change != SG::Change::kAppearance)
        ProcessPaneSizeChange(*this);
}

void TextPane::UpdateTextSize_() {
    ASSERT(text_node_);
    const auto size = text_node_->GetTextBounds().GetSize();
    text_size_.Set(size[0], size[1]);
    SetMinSize(ComputeMinSize());
}

void TextPane::SetTargetSize_(const Vector2f &size) {
    // Set the target_size in the LayoutOptions. A target_size of 1 in either
    // dimension means that the text fills its rectangle. A smaller value uses
    // just a fraction of the rectangle.  The other constraint is that the
    // text's aspect ratio should not change.
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
    Vector2f target_size;
    if (text_aspect <= size_aspect) {  // First picture.
        const float t0 = text_aspect * size[1] / size[0];
        target_size.Set(t0, 1);
    }
    else {                             // Second picture.
        const float t1 = size[0] / (text_aspect * size[1]);
        target_size.Set(1, t1);
    }
    auto &opts = text_node_->GetLayoutOptions();
    if (target_size != opts->GetTargetSize())
        opts->SetTargetSize(target_size);
}

void TextPane::SetTargetPoint_() {
    Point2f  target_point(0, 0);

    const HAlignment halign = halignment_;
    const VAlignment valign = valignment_;

    if      (halign == HAlignment::kAlignLeft)
        target_point[0] = -.5f;
    else if (halign == HAlignment::kAlignRight)
        target_point[0] =  .5f;

    if      (valign == VAlignment::kAlignBottom)
        target_point[1] = -.5f;
    else if (valign == VAlignment::kAlignTop)
        target_point[1] =  .5f;

    auto &opts = text_node_->GetLayoutOptions();
    if (target_point != opts->GetTargetPoint())
        opts->SetTargetPoint(target_point);
}

Vector2f TextPane::GetFixedSize_() const {
    const Vector2f base_size = GetBaseSize();
    return Vector2f(base_size[0] * text_size_[0],
                    base_size[1] * text_size_[1]);
}
