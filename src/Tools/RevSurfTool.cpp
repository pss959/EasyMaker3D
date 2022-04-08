#include "Tools/RevSurfTool.h"

#include "Managers/CommandManager.h"
#include "Models/RevSurfModel.h"
#include "Panels/RevSurfToolPanel.h"
#include "SG/Search.h"
#include "Util/Assert.h"

bool RevSurfTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<RevSurfModel>(sel);
}

void RevSurfTool::InitPanel() {
    auto model = Util::CastToDerived<RevSurfModel>(GetModelAttachedTo());
    ASSERT(model);
    auto &panel = GetTypedPanel<RevSurfToolPanel>();
    panel.SetProfile(model->GetProfile());
    panel.SetSweepAngle(model->GetSweepAngle());
}

void RevSurfTool::PanelChanged(const std::string &key,
                             ToolPanel::InteractionType type) {
    PanelTool::PanelChanged(key, type);

    auto &panel = GetTypedPanel<RevSurfToolPanel>();

    switch (type) {
      case ToolPanel::InteractionType::kDragStart:
        command_ = CreateCommand<ChangeRevSurfCommand>();
        command_->SetFromSelection(GetSelection());
        break;

      case ToolPanel::InteractionType::kDrag:
        ASSERT(command_);
        if (key == "Profile" || key == "SweepAngle") {
            command_->SetProfile(panel.GetProfile());
            command_->SetSweepAngle(panel.GetSweepAngle());
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
        command_ = CreateCommand<ChangeRevSurfCommand>();
        command_->SetFromSelection(GetSelection());
        command_->SetProfile(panel.GetProfile());
        command_->SetSweepAngle(panel.GetSweepAngle());
        GetContext().command_manager->AddAndDo(command_);
        command_.reset();
        break;
    }
}
