//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "App/RegisterTypes.h"

#include "App/ToolBox.h"
#include "Parser/Registry.h"
#include "SG/Init.h"

// ----------------------------------------------------------------------------
// These functions are found in separate files to make compilation faster.
// ----------------------------------------------------------------------------

extern void RegisterCommandTypes_();
extern void RegisterFeedbackTypes_();
extern void RegisterItemTypes_();
extern void RegisterModelTypes_();
extern void RegisterPaneTypes_();
extern void RegisterPanelTypes_();
extern void RegisterSGTypes_();
extern void RegisterTargetTypes_();
extern void RegisterToolTypes_();
extern void RegisterWidgetTypes_();

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

void RegisterTypes() {
    // Make it OK to call this more than once (mostly to simplify tests).
    if (Parser::Registry::GetTypeNameCount() > 0)
        return;

    // Make sure SG is initialized.
    SG::Init();

    RegisterCommandTypes_();
    RegisterFeedbackTypes_();
    RegisterItemTypes_();
    RegisterModelTypes_();
    RegisterPaneTypes_();
    RegisterPanelTypes_();
    RegisterSGTypes_();
    RegisterTargetTypes_();
    RegisterToolTypes_();
    RegisterWidgetTypes_();

    // Register the ToolBox specially.
    Parser::Registry::AddType<ToolBox>("ToolBox");
}

void UnregisterTypes() {
    Parser::Registry::Clear();
}
