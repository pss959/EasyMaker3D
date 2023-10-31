#include "Panes/ScrollingPane.h"

#include "Base/Event.h"
#include "Math/Linear.h"
#include "Panes/SliderPane.h"
#include "SG/CoordConv.h"
#include "SG/Search.h"
#include "Util/Tuning.h"

void ScrollingPane::AddFields() {
    AddField(contents_.Init("contents"));

    BoxPane::AddFields();
}

bool ScrollingPane::IsValid(Str &details) {
    if (! ContainerPane::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]
    if (! contents_.GetValue()) {
        details = "Missing contents pane";
        return false;
    }
    return true;
}

void ScrollingPane::CreationDone() {
    BoxPane::CreationDone();

    if (! IsTemplate()) {
        // Access the BoxPane that holds the contents and store the contents
        // Pane in it as a regular pane so that it can be handled normally.
        auto parent = FindTypedSubPane<BoxPane>("ContentsParent");
        parent->ReplacePanes(PaneVec(1, GetContentsPane()));

        // Hook up the slider.
        slider_pane_ = FindTypedSubPane<SliderPane>("Slider");
        slider_pane_->GetValueChanged().AddObserver(
            this, [&](float val){ ScrollTo(1 - val); });

        // Find and offset the thumb in Z.
        thumb_ = SG::FindNodeUnderNode(*this, "Thumb");
        thumb_->SetTranslation(Vector3f(0, 0, TK::kPaneZOffset));
    }
}

float ScrollingPane::GetScrollBarWidth() const {
    return FindSubPane("ScrollBar")->GetMinSize()[0];
}

void ScrollingPane::UpdateForLayoutSize(const Vector2f &size) {
    BoxPane::UpdateForLayoutSize(size);

    // Compute the scroll factor. If the unclipped size of the contents is not
    // larger than the size of the clip rectangle, then there is no
    // scrolling. Otherwise, it can scroll from 0 (with the top aligned with
    // the top of the clip rectangle) to 1 (with the bottom aligned with the
    // bottom of the clip rectangle). The distance to translate in the latter
    // case is the difference in sizes.
    const float clip_size = size[1];
    const float size_diff = std::max(0.f, GetContentsHeight_() - clip_size);
    scroll_factor_ = size_diff / clip_size;

    // Update the range of the thumb to fit the new size.
    const float thumb_height = thumb_->GetBounds().GetSize()[1];
    const float half_size = .5f - thumb_height / size[1];
    slider_pane_->SetNormalizedSliderRange(Vector2f(-half_size, half_size));

    // Enable or disable the thumb.
    slider_pane_->SetEnabled(size_diff > 0);

    UpdateScroll_();
}

ClickableWidgetPtr ScrollingPane::GetActivationWidget() const {
    // Nothing to click to activate the ScrollingPane.
    return nullptr;
}

BorderPtr ScrollingPane::GetFocusBorder() const {
    return GetBorder();
}

bool ScrollingPane::HandleEvent(const Event &event) {
    bool handled = false;

    // Handle up and down keys to scroll.
    if (event.flags.Has(Event::Flag::kKeyPress)) {
        const Str key_string = event.GetKeyString();
        if (key_string == "Up") {
            ScrollBy(-TK::kScrollingPaneKeyScrollAmount);
            handled = true;
        }
        else if (key_string == "Down") {
            ScrollBy(TK::kScrollingPaneKeyScrollAmount);
            handled = true;
        }
    }

    return handled;
}

void ScrollingPane::ScrollTo(float pos) {
    // Clamp just in case.
    scroll_pos_ = Clamp(pos, 0.f, 1.f);
    UpdateScroll_();
}

void ScrollingPane::ScrollBy(float amount) {
    // Compute a reasonable speed based on the scroll_factor_.
    const float speed =
        scroll_factor_ ? TK::kScrollingPaneSpeed / scroll_factor_ : 0.f;
    scroll_pos_ = Clamp(scroll_pos_ + speed * amount, 0.f, 1.f);
    UpdateScroll_();
}

void ScrollingPane::ScrollToShowSubPane(const Pane &sub_pane) {
    // Get the relative height of the sub-pane within the contents and convert
    // to a 0-1 fraction.
    const float pane_y = sub_pane.GetRelativePositionInParent()[1];
    const float fraction = -pane_y / GetContentsHeight_();

    // Try to put the pane in the middle.
    const float min = GetLayoutSize()[1] / GetContentsHeight_();
    ScrollTo(fraction <= min ? 0 : fraction >= 1 - min ? 1 : fraction);
}

Vector2f ScrollingPane::ComputeBaseSize() const {
    return GetMinSize();
}

void ScrollingPane::UpdateScroll_() {
    auto &contents = GetContentsPane();
    Vector2f trans = contents->GetContentsOffset();
    trans[1] = scroll_pos_ * scroll_factor_;
    contents->SetContentsOffset(trans);
    // Note that this does not notify observers, so it is safe to call even if
    // the slider was dragged.
    slider_pane_->SetValue(1 - scroll_pos_);
}

float ScrollingPane::GetContentsHeight_() const {
    // Remove padding from ScrollingPane and Contents Panes.
    const auto &contents = *GetContentsPane();
    return contents.GetUnclippedSize()[1] -
        2 * (GetPadding() + contents.GetPadding());
}
