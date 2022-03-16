#include "Panes/ScrollingPane.h"

#include "Event.h"
#include "Math/Linear.h"

void ScrollingPane::AddFields() {
    AddField(contents_);
    ContainerPane::AddFields();
}

bool ScrollingPane::IsValid(std::string &details) {
    if (! ContainerPane::IsValid(details))
        return false;

    if (! contents_.GetValue()) {
        details = "Missing contents pane";
        return false;
    }
    if (! GetPanes().empty()) {
        details = "Panes field must be empty";
        return false;
    }

    return true;
}

void ScrollingPane::CreationDone() {
    BoxPane::CreationDone();

    if (! IsTemplate()) {
        // Store the contents Pane as a regular pane so that it can be handled
        // normally.
        ReplacePanes(std::vector<PanePtr>(1, GetContentsPane()));

        // The panes field should not be written.
        HidePanesField();
    }
}

void ScrollingPane::SetSize(const Vector2f &size) {
    BoxPane::SetSize(size);

    // Compute the scroll factor.  If the unclipped size of the contents is not
    // larger than the size of the clip rectangle, then there is no
    // scrolling. Otherwise, it can scroll from 0 (with the top aligned with
    // the top of the clip rectangle) to 1 (with the bottom aligned with the
    // bottom of the clip rectangle). The distance to translate in the latter
    // case is the difference in sizes.
    auto &contents = GetContentsPane();
    const float clip_size = GetSize()[1];
    const float size_diff =
        std::max(0.f, contents->GetUnclippedSize()[1] - clip_size);
    scroll_factor_ = size_diff / clip_size;

    UpdateScroll_();
}

bool ScrollingPane::HandleEvent(const Event &event) {
    bool handled = false;
    if (event.flags.Has(Event::Flag::kKeyPress)) {
        const std::string key_string = event.GetKeyString();
        if (key_string == "Up") {
            ScrollBy(-.5f);
            handled = true;
        }
        else if (key_string == "Down") {
            ScrollBy(.5f);
            handled = true;
        }
    }
    return handled;
}

void ScrollingPane::ScrollToTop() {
    scroll_pos_ = 0;
    UpdateScroll_();
}

void ScrollingPane::ScrollBy(float amount) {
    // Compute a reasonable speed based on the scroll_factor_.
    const float speed = scroll_factor_ ? .4f / scroll_factor_ : 0.f;
    scroll_pos_ = Clamp(scroll_pos_ + speed * amount, 0.f, 1.f);
    UpdateScroll_();
}

void ScrollingPane::UpdateScroll_() {
    auto &contents = GetContentsPane();
    Vector2f trans = contents->GetContentsOffset();
    trans[1] = scroll_pos_ * scroll_factor_;
    contents->SetContentsOffset(trans);
}
