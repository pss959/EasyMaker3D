#include "Panes/ScrollingPane.h"

#include "Base/Event.h"
#include "Base/Tuning.h"
#include "Math/Linear.h"
#include "Panes/SliderPane.h"
#include "SG/Search.h"

void ScrollingPane::AddFields() {
    AddField(contents_.Init("contents"));

    BoxPane::AddFields();
}

bool ScrollingPane::IsValid(std::string &details) {
    if (! ContainerPane::IsValid(details))
        return false;
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
        auto parent = FindTypedPane<BoxPane>("ContentsParent");
        parent->ReplacePanes(std::vector<PanePtr>(1, GetContentsPane()));

        // Hook up the slider.
        slider_pane_ = FindTypedPane<SliderPane>("Slider");
        slider_pane_->GetValueChanged().AddObserver(
            this, [&](float val){ ScrollTo(1 - val); });
    }
}

float ScrollingPane::GetScrollBarWidth() const {
    return FindPane("ScrollBar")->GetMinSize()[0];
}

void ScrollingPane::SetLayoutSize(const Vector2f &size) {
    BoxPane::SetLayoutSize(size);

    // Compute the scroll factor.  If the unclipped size of the contents is not
    // larger than the size of the clip rectangle, then there is no
    // scrolling. Otherwise, it can scroll from 0 (with the top aligned with
    // the top of the clip rectangle) to 1 (with the bottom aligned with the
    // bottom of the clip rectangle). The distance to translate in the latter
    // case is the difference in sizes.
    auto &contents = GetContentsPane();
    const float clip_size = size[1];
    const float size_diff =
        std::max(0.f, contents->GetUnclippedSize()[1] - clip_size);
    scroll_factor_ = size_diff / clip_size;

    // Update the range of the thumb to fit the new size.
    const float thumb_height =
        SG::FindNodeUnderNode(*this, "Thumb")->GetBounds().GetSize()[1];
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

bool ScrollingPane::CanFocus() const {
    return true;
}

bool ScrollingPane::HandleEvent(const Event &event) {
    bool handled = false;

    // Handle up and down keys to scroll.
    if (event.flags.Has(Event::Flag::kKeyPress)) {
        const std::string key_string = event.GetKeyString();
        if (key_string == "Up") {
            ScrollBy(-TK::kScrollingPaneKeyScrollAmount);
            handled = true;
        }
        else if (key_string == "Down") {
            ScrollBy(TK::kScrollingPaneKeyScrollAmount);
            handled = true;
        }
    }

    // Valuator events scroll as well.
    if (event.flags.Has(Event::Flag::kPosition1D)) {
        ScrollBy(TK::kScrollingPaneWheelScrollAmount * event.position1D);
        handled = true;
    }

    return handled;
}

void ScrollingPane::AddEnabledWidgets(std::vector<WidgetPtr> &widgets) const {
    // Do not add widgets that are clipped away.
    std::cerr << "XXXX In ScrollingPane::AddEnabledWidgets for "
              << GetDesc() << "\n";

    // XXXX Call contents to add all widgets?
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

void ScrollingPane::UpdateScroll_() {
    auto &contents = GetContentsPane();
    Vector2f trans = contents->GetContentsOffset();
    trans[1] = scroll_pos_ * scroll_factor_;
    contents->SetContentsOffset(trans);
    // Note that this does not notify observers, so it is safe to call even if
    // the slider was dragged.
    slider_pane_->SetValue(1 - scroll_pos_);
}
