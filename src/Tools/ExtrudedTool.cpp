#include "Tools/ExtrudedTool.h"

#include "Managers/CommandManager.h"
#include "Math/Linear.h"
#include "Models/ExtrudedModel.h"
#include "Panels/ExtrudedToolPanel.h"
#include "Place/PrecisionStore.h"
#include "SG/Search.h"
#include "Util/Assert.h"

bool ExtrudedTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<ExtrudedModel>(sel);
}

void ExtrudedTool::Attach() {
    PanelTool::Attach();
    GetContext().precision_store->GetChanged().AddObserver(
        this, [&](){ UpdatePrecision_(); });
}

void ExtrudedTool::Detach() {
    GetContext().precision_store->GetChanged().RemoveObserver(this);
    PanelTool::Detach();
}

void ExtrudedTool::InitPanel() {
    auto model = Util::CastToDerived<ExtrudedModel>(GetModelAttachedTo());
    ASSERT(model);
    auto &panel = GetTypedPanel<ExtrudedToolPanel>();
    panel.SetProfile(model->GetProfile());
    UpdatePrecision_();
}

void ExtrudedTool::PanelChanged(const std::string &key,
                                ToolPanel::InteractionType type) {
    PanelTool::PanelChanged(key, type);

    auto &panel = GetTypedPanel<ExtrudedToolPanel>();

    switch (type) {
        using enum ToolPanel::InteractionType;
      case kDragStart:
        command_ = CreateCommand<ChangeExtrudedCommand>();
        command_->SetFromSelection(GetSelection());
        break;

      case kDrag:
        ASSERT(command_);
        if (key == "Profile") {
            command_->SetProfile(panel.GetProfile());
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
        command_ = CreateCommand<ChangeExtrudedCommand>();
        command_->SetFromSelection(GetSelection());
        command_->SetProfile(panel.GetProfile());
        GetContext().command_manager->AddAndDo(command_);
        command_.reset();
        break;
    }
}

void ExtrudedTool::UpdatePrecision_() {
    const float prec = GetContext().precision_store->GetLinearPrecision();
    const Vector3f &scale = GetModelAttachedTo()->GetScale();

    // Compute the scaled precision in the profile space (0-1). For example, if
    // the X scale is 8, a precision value of 1 is 1/8th the width of the
    // profile area.
    auto &panel = GetTypedPanel<ExtrudedToolPanel>();
    panel.SetPrecision(prec / ToVector2f(scale));
}
