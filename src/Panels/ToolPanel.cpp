#include "Panels/ToolPanel.h"

ToolPanel::ToolPanel() {
}

bool ToolPanel::IsCloseable() const {
    return false;
}

void ToolPanel::ReportChange(const Str &key, InteractionType type) {
    interaction_.Notify(key, type);
}

bool ToolPanel::ShouldTrapValuatorEvents() const {
    return false;
}

void ToolPanel::Close(const Str &result) {
    if (IsCloseable())
        Panel::Close(result);
}
