#include "Panels/ToolPanel.h"

ToolPanel::ToolPanel() {
}

void ToolPanel::ReportChange(const Str &key, InteractionType type) {
    interaction_.Notify(key, type);
}

void ToolPanel::Close(const Str &result) {
    if (IsCloseable())
        Panel::Close(result);
}
