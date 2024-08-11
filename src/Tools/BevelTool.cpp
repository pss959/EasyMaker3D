//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tools/BevelTool.h"

#include "Managers/CommandManager.h"
#include "Models/BeveledModel.h"
#include "Panels/BevelToolPanel.h"
#include "Util/Assert.h"

bool BevelTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<BeveledModel>(sel);
}

void BevelTool::InitPanel() {
    auto model = std::dynamic_pointer_cast<BeveledModel>(GetModelAttachedTo());
    ASSERT(model);
    GetTypedPanel<BevelToolPanel>().SetBevel(model->GetBevel());
}

void BevelTool::PanelChanged(const Str &key, ToolPanel::InteractionType type) {
    PanelTool::PanelChanged(key, type);

    BevelToolPanel &panel = GetTypedPanel<BevelToolPanel>();

    switch (type) {
        using enum ToolPanel::InteractionType;
      case kDragStart:
        command_ = Command::CreateCommand<ChangeBevelCommand>();
        command_->SetFromSelection(GetSelection());
        start_bevel_ = panel.GetBevel();
        break;

      case kDrag:
        ASSERT(command_);
        if (key == "Profile" || key == "Scale" || key == "MaxAngle") {
            command_->SetBevel(panel.GetBevel());
            // Simulate execution to update all the Models.
            GetContext().command_manager->SimulateDo(command_);
        }
        break;

      case kDragEnd:
        ASSERT(command_);
        // Don't process the command if the bevel was not set.
        if (panel.GetBevel() != start_bevel_)
            GetContext().command_manager->AddAndDo(command_);
        command_.reset();
        break;

      case kImmediate:
        // User clicked on profile to add a point.
        ASSERT(key == "Profile");
        command_ = Command::CreateCommand<ChangeBevelCommand>();
        command_->SetFromSelection(GetSelection());
        command_->SetBevel(panel.GetBevel());
        GetContext().command_manager->AddAndDo(command_);
        command_.reset();
        break;
    }
}
