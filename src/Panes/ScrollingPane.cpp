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
    // Store the contents Pane as a regular pane so that it can be handled
    // normally.
    ReplacePanes(std::vector<PanePtr>(1, GetContentsPane()));

    // The panes field should not be written.
    HidePanesField();
}

void ScrollingPane::SetSize(const Vector2f &size) {
    ContainerPane::SetSize(size);
    if (const ContainerPanePtr &contents = GetContentsPane()) {
        const Vector2f contents_min = contents->GetMinSize();
        contents->SetSize(MaxComponents(contents_min, size));
    }
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

void ScrollingPane::Scroll(float amount) {
    std::cerr << "XXXX SCROLL by " << amount << "\n";
    if (const ContainerPanePtr &contents = GetContentsPane()) {
        Vector3f trans = contents->GetTranslation();
        trans[1] += .01f * amount;
        contents->SetTranslation(trans);
    }
}

Vector2f ScrollingPane::ComputeMinSize() const {
    // Do not inherit from BoxPane. Use the default Pane version. This ensures
    // that the size of the ScrollingPane is not affected by its contents.
    return Pane::ComputeMinSize();
}
