#include "Panels/ToolPanel.h"

ToolPanel::ToolPanel() {
}

bool ToolPanel::IsCloseable() const {
    return false;
}

void ToolPanel::ReportChange(const std::string &key, InteractionType type) {
    interaction_.Notify(key, type);
}

bool ToolPanel::ShouldTrapValuatorEvents() const {
    return false;
}

void ToolPanel::Close(const std::string &result) {
    std::cerr << "XXXX Closing " << GetDesc()
              << " IsCloseable = " << IsCloseable() << "\n";
    if (IsCloseable())
        Panel::Close(result);
}
