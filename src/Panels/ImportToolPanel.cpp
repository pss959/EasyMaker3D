#include "Panels/ImportToolPanel.h"

void ImportToolPanel::DisplayImportError(const std::string &message) {
    DisplayMessage(message, nullptr);
}

void ImportToolPanel::ProcessResult(const std::string &result) {
    ReportChange(result, InteractionType::kImmediate);
}
