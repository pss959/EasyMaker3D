#include "Panels/ImportToolPanel.h"

void ImportToolPanel::DisplayImportError(const Str &message) {
    DisplayMessage(message, nullptr);
}

void ImportToolPanel::ProcessResult(const Str &result) {
    ReportChange(result, InteractionType::kImmediate);
}
