//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tools/CSGTool.h"

#include "Commands/ChangeCSGOperationCommand.h"
#include "Enums/CSGOperation.h"
#include "Managers/CommandManager.h"
#include "Models/CSGModel.h"
#include "Panels/CSGToolPanel.h"
#include "Util/Assert.h"

bool CSGTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<CSGModel>(sel);
}

void CSGTool::InitPanel() {
    auto model = std::dynamic_pointer_cast<CSGModel>(GetModelAttachedTo());
    ASSERT(model);
    GetTypedPanel<CSGToolPanel>().SetOperation(model->GetOperation());
}

void CSGTool::PanelChanged(const Str &key, ToolPanel::InteractionType type) {
    // The CSGToolPanel has only one type of change.
    ASSERT(key == "Operation");
    ASSERT(type == ToolPanel::InteractionType::kImmediate);

    CSGToolPanel &panel = GetTypedPanel<CSGToolPanel>();

    auto command = Command::CreateCommand<ChangeCSGOperationCommand>();
    command->SetFromSelection(GetSelection());
    command->SetNewOperation(panel.GetOperation());
    GetContext().command_manager->AddAndDo(command);
}
