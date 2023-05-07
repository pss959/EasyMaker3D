#include "Tools/TaperTool.h"

#include "Managers/CommandManager.h"
#include "Models/TaperedModel.h"
#include "Panels/TaperToolPanel.h"
#include "SG/Search.h"
#include "Util/Assert.h"

bool TaperTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<TaperedModel>(sel);
}

void TaperTool::InitPanel() {
    auto model = Util::CastToDerived<TaperedModel>(GetModelAttachedTo());
    ASSERT(model);
    GetTypedPanel<TaperToolPanel>().SetTaper(model->GetTaper());
}

void TaperTool::PanelChanged(const std::string &key,
                             ToolPanel::InteractionType type) {
    PanelTool::PanelChanged(key, type);

    TaperToolPanel &panel = GetTypedPanel<TaperToolPanel>();

    switch (type) {
      case ToolPanel::InteractionType::kDragStart:
        command_ = CreateCommand<ChangeTaperCommand>();
        command_->SetFromSelection(GetSelection());
        start_taper_ = panel.GetTaper();
        break;

      case ToolPanel::InteractionType::kDrag:
        ASSERT(command_);
        if (key == "Profile" || key == "Scale" || key == "MaxAngle") {
            command_->SetTaper(panel.GetTaper());
            // Simulate execution to update all the Models.
            GetContext().command_manager->SimulateDo(command_);
        }
        break;

      case ToolPanel::InteractionType::kDragEnd:
        ASSERT(command_);
        // Don't process the command if the taper was not set.
        if (panel.GetTaper() != start_taper_)
            GetContext().command_manager->AddAndDo(command_);
        command_.reset();
        break;

      case ToolPanel::InteractionType::kImmediate:
        // User clicked on profile to add a point.
        ASSERT(key == "Profile");
        command_ = CreateCommand<ChangeTaperCommand>();
        command_->SetFromSelection(GetSelection());
        command_->SetTaper(panel.GetTaper());
        GetContext().command_manager->AddAndDo(command_);
        command_.reset();
        break;
    }
}
