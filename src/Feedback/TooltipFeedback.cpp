//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Feedback/TooltipFeedback.h"

#include <algorithm>

#include <ion/math/vectorutils.h>

#include "Items/Border.h"
#include "Math/Color.h"
#include "Math/Linear.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "Util/Delay.h"
#include "Util/Tuning.h"

// ----------------------------------------------------------------------------
// TooltipFeedback::Impl_ class definition.
// ----------------------------------------------------------------------------

class TooltipFeedback::Impl_ {
  public:
    Impl_(SG::Node &root_node) : root_node_(root_node) {}

    static void SetDelay(float seconds) { delay_ = seconds; }

    void InitParts();
    void SetText(const Str &text);
    Vector3f GetTextSize() const;
    void Activate();
    void Deactivate();

  private:
    SG::Node        &root_node_;
    SG::NodePtr      background_;
    SG::TextNodePtr  text_;
    BorderPtr        border_;

    /// Amount to delay showing any tooltip.
    static float delay_;

    /// Set to true while tooltip is being delayed.
    bool is_delayed_ = false;

    /// ID returned by Util::RunDelayed(); used to cancel.
    int delay_id_;

    /// Shows or hides the tooltip.
    void Show_(bool show);
};

// ----------------------------------------------------------------------------
// TooltipFeedback::Impl_ functions.
// ----------------------------------------------------------------------------

float TooltipFeedback::Impl_::delay_ = TK::kTooltipDelay;

void TooltipFeedback::Impl_::InitParts() {
    text_       = SG::FindTypedNodeUnderNode<SG::TextNode>(root_node_, "Text");
    background_ = SG::FindNodeUnderNode(root_node_, "Background");
    border_     = SG::FindTypedNodeUnderNode<Border>(root_node_, "Border");
}

void TooltipFeedback::Impl_::SetText(const Str &text) {
    text_->SetText(text);

    // Compute the number of lines in the text and use that to scale it to
    // maintain a constant size.
    const size_t line_count = 1 + std::count(text.begin(), text.end(), '\n');
    text_->SetUniformScale(line_count);

    const Vector3f size = GetTextSize();
    background_->SetScale(size);
    border_->SetSize(ToVector2f(size));
}

Vector3f TooltipFeedback::Impl_::GetTextSize() const {
    Vector2f size = text_->GetTextSize();
    size[0] *= 1.1f;
    size[1] *= 1.6f;
    return text_->GetScale() * Vector3f(size, 1);
}

void TooltipFeedback::Impl_::Activate() {
    if (! is_delayed_ && delay_ > 0) {
        delay_id_ = Util::RunDelayed(delay_, [this](){ Show_(true); });
        is_delayed_ = true;
    }
}

void TooltipFeedback::Impl_::Deactivate() {
    if (is_delayed_) {
        is_delayed_ = false;
        Util::CancelDelayed(delay_id_);
    }
    Show_(false);
}

void TooltipFeedback::Impl_::Show_(bool show) {
    root_node_.SetEnabled(show);
}

// ----------------------------------------------------------------------------
// TooltipFeedback functions.
// ----------------------------------------------------------------------------

TooltipFeedback::TooltipFeedback() : impl_(new Impl_(*this)) {
}

void TooltipFeedback::CreationDone() {
    Feedback::CreationDone();
    if (! IsTemplate())
        impl_->InitParts();
}

void TooltipFeedback::SetDelay(float seconds) {
    Impl_::SetDelay(seconds);
}

void TooltipFeedback::SetText(const Str &text) {
    impl_->SetText(text);
}

Vector3f TooltipFeedback::GetTextSize() const {
    return impl_->GetTextSize();
}

void TooltipFeedback::Activate() {
    impl_->Activate();
}

void TooltipFeedback::Deactivate() {
    impl_->Deactivate();
}
