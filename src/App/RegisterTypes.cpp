#include "App/RegisterTypes.h"

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
}

void UnregisterTypes() {
    Parser::Registry::Clear();
}
