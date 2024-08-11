//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Panels/ImportToolPanel.h"

void ImportToolPanel::DisplayImportError(const Str &message) {
    DisplayMessage(message);
}

void ImportToolPanel::ProcessResult(const Str &result) {
    ReportChange(result, InteractionType::kImmediate);
}
