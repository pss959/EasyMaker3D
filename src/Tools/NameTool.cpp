#include "Tools/NameTool.h"

#include "Commands/ChangeNameCommand.h"
#include "Managers/CommandManager.h"
#include "Models/Model.h"
#include "Panels/NameToolPanel.h"
#include "Util/Assert.h"

bool NameTool::CanAttach(const Selection &sel) const {
    // There has to be exactly one selected Model.
    return sel.GetCount() == 1U;
}

void NameTool::InitPanel() {
    ASSERT(GetModelAttachedTo());
    GetTypedPanel<NameToolPanel>().SetName(GetModelAttachedTo()->GetName());
}

void NameTool::PanelChanged(const Str &key, ToolPanel::InteractionType type) {
    // The NameToolPanel has only one type of change.
    ASSERT(key == "Name");
    ASSERT(type == ToolPanel::InteractionType::kImmediate);

    NameToolPanel &panel = GetTypedPanel<NameToolPanel>();

    auto command = Command::CreateCommand<ChangeNameCommand>();
    command->SetFromSelection(GetSelection());
    command->SetNewName(panel.GetName());
    GetContext().command_manager->AddAndDo(command);
}
