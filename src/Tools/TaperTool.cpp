#include "Tools/TaperTool.h"

#include "Managers/CommandManager.h"
#include "Models/TaperedModel.h"
#include "Panels/TaperToolPanel.h"
#include "Util/Assert.h"

bool TaperTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<TaperedModel>(sel);
}

void TaperTool::InitPanel() {
    auto model = std::dynamic_pointer_cast<TaperedModel>(GetModelAttachedTo());
    ASSERT(model);
    GetTypedPanel<TaperToolPanel>().SetTaper(model->GetTaper());
}

void TaperTool::PanelChanged(const std::string &key,
                             ToolPanel::InteractionType type) {
    PanelTool::PanelChanged(key, type);

    TaperToolPanel &panel = GetTypedPanel<TaperToolPanel>();

    switch (type) {
        using enum ToolPanel::InteractionType;
      case kDragStart:
        command_ = CreateCommand<ChangeTaperCommand>();
        command_->SetFromSelection(GetSelection());
        start_taper_ = panel.GetTaper();
        break;

      case kDrag:
        ASSERT(command_);
        ASSERT(key == "Profile");
        command_->SetTaper(panel.GetTaper());
        // Simulate execution to update all the Models.
        GetContext().command_manager->SimulateDo(command_);
        break;

      case kDragEnd:
        ASSERT(command_);
        // Don't process the command unless something changed.
        if (panel.GetTaper() != start_taper_)
            GetContext().command_manager->AddAndDo(command_);
        command_.reset();
        break;

      case kImmediate:
        // User changed the axis or clicked on profile to add a point.
        ASSERT(key == "Axis" || key == "Profile");
        command_ = CreateCommand<ChangeTaperCommand>();
        command_->SetFromSelection(GetSelection());
        command_->SetTaper(panel.GetTaper());
        GetContext().command_manager->AddAndDo(command_);
        command_.reset();
        break;
    }
}
