#pragma once

#include "Commands/ChangeCylinderCommand.h"
#include "Feedback/LinearFeedback.h"
#include "Models/CylinderModel.h"
#include "Tools/Tool.h"
#include "Widgets/ScaleWidget.h"

/// CylinderTool is a specialized Tool that is used to edit the top and bottom
/// radii of a CylinderModel.
///
/// \ingroup Tools
class CylinderTool : public Tool {
  public:
    virtual bool IsSpecialized() const override { return true; }

    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual void UpdateGripInfo(GripInfo &info) override;

  protected:
    CylinderTool();

    virtual void CreationDone() override;

    virtual bool CanAttach(const Selection &sel) const override;
    virtual void Attach() override;
    virtual void Detach() override;

  private:
    ScaleWidgetPtr    top_scaler_;
    ScaleWidgetPtr    bottom_scaler_;
    LinearFeedbackPtr feedback_;

    /// Changing radius at the start of a drag.
    float start_radius_ = 0;

    /// Attached CylinderModel.
    CylinderModelPtr cylinder_model_;

    /// Command used to modify the CylinderModel radius.
    ChangeCylinderCommandPtr command_;

    /// Initializes and returns a ScaleWidget scaler.
    ScaleWidgetPtr InitScaler_(const Str &name);

    /// Updates both scalers based on the attached CylinderModel.
    void UpdateScalers_();

    /// Scales the handles and stick of a ScaleWidget based on the Model size.
    void ScaleScaler_(ScaleWidget &scaler, const Vector3f &model_size);

    void ScalerActivated_(const ScaleWidgetPtr &scaler, bool is_activation);
    void ScalerChanged_(const ScaleWidgetPtr &scaler, bool is_max);

    void UpdateFeedback_(float radius, bool is_snapped);

    friend class Parser::Registry;
};
