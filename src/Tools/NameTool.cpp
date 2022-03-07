#include "Tools/NameTool.h"

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

void NameTool::PanelChanged(const std::string &key,
                            ToolPanel::InteractionType type) {
#if XXXX
    command_ = CreateCommand<ChangeNameCommand>("ChangeNameCommand");
    command_->SetFromSelection(GetSelection());
    command_->SetName(panel.GetName());
    GetContext().command_manager->AddAndDo(command_);
    command_.reset();
#endif
}
