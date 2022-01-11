#include "Panes/ScrollingPane.h"

#include "Event.h"
#include "Math/Linear.h"

void ScrollingPane::SetSize(const Vector2f &size) {
    BoxPane::SetSize(size);
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

void ScrollingPane::UpdateScroll_() {
    // When scroll_pos_ is 0, this should put the top of the contents box at
    // the top of the ScrollingPane. When it is 1, this should put the bottom
    // of the contents at the bottom, but only if the contents are taller than
    // the ScrollingPane.
    auto contents = this; // XXXX GetContentsPane();
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
