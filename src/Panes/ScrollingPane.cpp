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

void ScrollingPane::AllFieldsParsed(bool is_template) {
    ContainerPane::AllFieldsParsed(is_template);

    // Store the contents Pane as a regular pane so that it can be handled
    // normally.
    ReplacePanes(std::vector<PanePtr>(1, GetContentsPane()));

    // The panes field should not be written.
    HidePanesField();
}

void ScrollingPane::SetSize(const Vector2f &size) {
    ContainerPane::SetSize(size);
    auto &contents = GetContentsPane();
    const Vector2f contents_min = contents->GetMinSize();
    const Vector2f contents_size = MaxComponents(contents_min, size);
    contents->SetSize(contents_size);

    // Clip to the size of the ScrollingPane.
    // XXXX contents->SetClipSize(size);

    // Scale the contents so that the relative size of everything in it remains
    // the same and translate to the relative position.
    contents->SetScale(Vector3f(1, contents_size[1] / size[1], 1));
    UpdateScroll_();
}

bool ScrollingPane::HandleEvent(const Event &event) {
    bool handled = false;
    if (event.flags.Has(Event::Flag::kKeyPress)) {
        const std::string key_string = event.GetKeyString();
        if (key_string == "Up") {
            std::cerr << "XXXX ScrollingPane got UP\n";
            handled = true;
        }
        else if (key_string == "Down") {
            std::cerr << "XXXX ScrollingPane got DOWN\n";
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
    const float kScrollMult = .02f;
    scroll_pos_ = Clamp(scroll_pos_ + kScrollMult * amount, 0.f, 1.f);
    UpdateScroll_();
}

Vector2f ScrollingPane::ComputeMinSize() const {
    // Do not inherit from BoxPane. Use the default Pane version. This ensures
    // that the size of the ScrollingPane is not affected by its contents.
    return Pane::ComputeMinSize();
}

void ScrollingPane::UpdateScroll_() {
    // When scroll_pos_ is 0, this should put the top of the contents box at
    // the top of the ScrollingPane. When it is 1, this should put the bottom
    // of the contents at the bottom, but only if the contents are taller than
    // the ScrollingPane.
    auto &contents = GetContentsPane();
    const float size = GetSize()[1];
    const float size_diff = contents->GetMinSize()[1] - size;
    Vector3f trans = contents->GetTranslation();
    if (size_diff > 0) {
        const float offset = size_diff / GetSize()[1];
        trans[1] = -offset / 2 + offset * scroll_pos_;
    }
    else {
        // No scrolling. Leave the top of the contents at the top.
        trans[1] = 0;
    }
    contents->SetTranslation(trans);
}
