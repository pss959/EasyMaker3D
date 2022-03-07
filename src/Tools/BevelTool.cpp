#include "Tools/BevelTool.h"

#include "Managers/CommandManager.h"
#include "Models/BeveledModel.h"
#include "Panels/BevelToolPanel.h"
#include "SG/Search.h"
#include "Util/Assert.h"

bool BevelTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<BeveledModel>(sel);
}

void BevelTool::InitPanel() {
    auto model = Util::CastToDerived<BeveledModel>(GetModelAttachedTo());
    ASSERT(model);
    GetTypedPanel<BevelToolPanel>().SetBevel(model->GetBevel());
}

void BevelTool::PanelChanged(const std::string &key,
                             ToolPanel::InteractionType type) {
    PanelTool::PanelChanged(key, type);

    BevelToolPanel &panel = GetTypedPanel<BevelToolPanel>();

    switch (type) {
      case ToolPanel::InteractionType::kDragStart:
        command_ = CreateCommand<ChangeBevelCommand>("ChangeBevelCommand");
        command_->SetFromSelection(GetSelection());
        break;

      case ToolPanel::InteractionType::kDrag:
        ASSERT(command_);
        if (key == "Profile" || key == "Scale" || key == "MaxAngle") {
            command_->SetBevel(panel.GetBevel());
            // Simulate execution to update all the Models.
            GetContext().command_manager->SimulateDo(command_);
        }
        break;

      case ToolPanel::InteractionType::kDragEnd:
        ASSERT(command_);
        GetContext().command_manager->AddAndDo(command_);
        command_.reset();
        break;

      case ToolPanel::InteractionType::kImmediate:
        // User clicked on profile to add a point.
        ASSERT(key == "Profile");
        command_ = CreateCommand<ChangeBevelCommand>("ChangeBevelCommand");
        command_->SetFromSelection(GetSelection());
        command_->SetBevel(panel.GetBevel());
        GetContext().command_manager->AddAndDo(command_);
        command_.reset();
        break;
    }
}
