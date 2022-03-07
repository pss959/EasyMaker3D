﻿#pragma once

#include <memory>

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
    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual GripGuideType GetGripGuideType() const override {
        return GripGuideType::kBasic;
    }
    virtual void UpdateGripInfo(GripInfo &info) override;

  protected:
    CylinderTool();

    virtual void CreationDone() override;

    virtual bool IsSpecialized() const override { return true; }
    virtual bool CanAttach(const Selection &sel) const override;
    virtual void Attach() override;
    virtual void Detach() override;

  private:
    static constexpr float kMinRadius_ = .01f;
    static constexpr float kMaxRadius_ = 30;

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
    ScaleWidgetPtr InitScaler_(const std::string &name);

    /// Updates both scalers based on the attached CylinderModel.
    void UpdateScalers_();

    void ScalerActivated_(const ScaleWidgetPtr &scaler, bool is_activation);
    void ScalerChanged_(const ScaleWidgetPtr &scaler, bool is_max);

    void UpdateFeedback_(float radius, bool is_snapped);

    friend class Parser::Registry;
};
