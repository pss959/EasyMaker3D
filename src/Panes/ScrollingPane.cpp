#include "Panes/ScrollingPane.h"

#include "Event.h"

void ScrollingPane::AddFields() {
    // XXXX AddField(XXXX);
    BoxPane::AddFields();
}

void ScrollingPane::SetPanes(const std::vector<PanePtr> &new_panes) {
    ReplacePanes(new_panes);
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

Vector2f ScrollingPane::ComputeMinSize() const {
    // Do not inherit from BoxPane. Use the default Pane version.
    return Pane::ComputeMinSize();
}
