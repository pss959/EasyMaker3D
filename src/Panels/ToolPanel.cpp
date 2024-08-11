//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

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
