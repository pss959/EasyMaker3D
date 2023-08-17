#include "Tools/RevSurfTool.h"

#include "Managers/CommandManager.h"
#include "Math/Linear.h"
#include "Models/RevSurfModel.h"
#include "Panels/RevSurfToolPanel.h"
#include "Place/PrecisionStore.h"
#include "SG/Search.h"
#include "Util/Assert.h"

bool RevSurfTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<RevSurfModel>(sel);
}

void RevSurfTool::Attach() {
    PanelTool::Attach();
    GetContext().precision_store->GetChanged().AddObserver(
        this, [&](){ UpdatePrecision_(); });
}

void RevSurfTool::Detach() {
    GetContext().precision_store->GetChanged().RemoveObserver(this);
    PanelTool::Detach();
}

void RevSurfTool::InitPanel() {
    auto model = std::dynamic_pointer_cast<RevSurfModel>(GetModelAttachedTo());
    ASSERT(model);
    auto &panel = GetTypedPanel<RevSurfToolPanel>();
    panel.SetProfile(model->GetProfile());
    panel.SetSweepAngle(model->GetSweepAngle());
    UpdatePrecision_();
}

void RevSurfTool::PanelChanged(const Str &key,
                               ToolPanel::InteractionType type) {
    PanelTool::PanelChanged(key, type);

    auto &panel = GetTypedPanel<RevSurfToolPanel>();

    switch (type) {
        using enum ToolPanel::InteractionType;
      case kDragStart:
        command_ = Command::CreateCommand<ChangeRevSurfCommand>();
        command_->SetFromSelection(GetSelection());
        break;

      case kDrag:
        ASSERT(command_);
        if (key == "Profile" || key == "SweepAngle") {
            command_->SetProfile(panel.GetProfile());
            command_->SetSweepAngle(panel.GetSweepAngle());
            // Simulate execution to update all the Models.
            GetContext().command_manager->SimulateDo(command_);
            point_dragged_ = true;
        }
        break;

      case kDragEnd:
        ASSERT(command_);
        // Don't do anything if there was no actual drag (i.e., the point was
        // clicked).
        if (point_dragged_) {
            GetContext().command_manager->AddAndDo(command_);
            point_dragged_ = false;
        }
        command_.reset();
        break;

      case kImmediate:
        // User clicked on profile to add a point.
        ASSERT(key == "Profile");
        command_ = Command::CreateCommand<ChangeRevSurfCommand>();
        command_->SetFromSelection(GetSelection());
        command_->SetProfile(panel.GetProfile());
        command_->SetSweepAngle(panel.GetSweepAngle());
        GetContext().command_manager->AddAndDo(command_);
        command_.reset();
        break;
    }
}

void RevSurfTool::UpdatePrecision_() {
    const float prec = GetContext().precision_store->GetLinearPrecision();
    const Vector3f &scale = GetModelAttachedTo()->GetScale();

    // Compute the scaled precision in the profile space (0-1). For example, if
    // the X scale is 8, a precision value of 1 is 1/8th the width of the
    // profile area.
    auto &panel = GetTypedPanel<RevSurfToolPanel>();
    panel.SetPrecision(prec / ToVector2f(scale));
}
