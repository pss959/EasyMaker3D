#pragma once

#include <memory>

// XXXX #include "Commands/ChangeCylinderCommand.h"
#include "Feedback/LinearFeedback.h"
#include "Models/CylinderModel.h"
#include "Tools/SpecializedTool.h"
#include "Widgets/RangeWidget.h"

/// CylinderTool is a SpecializedTool that is used to edit the top and bottom
/// radii of a CylinderModel.
/// \ingroup Tools
class CylinderTool : public SpecializedTool {
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

    virtual bool CanAttachToModel(const Model &model) const override;
    virtual void Attach() override;
    virtual void Detach() override;

  private:
    static constexpr float kMinRadius_ = .01f;
    static constexpr float kMaxRadius_ = 20;

    RangeWidgetPtr    top_scaler_;
    RangeWidgetPtr    bottom_scaler_;
    LinearFeedbackPtr feedback_;

    /// Changing radius at the start of a drag.
    float start_radius_;

    /// Attached CylinderModel.
    CylinderModelPtr cylinder_model_;

    /// Command used to modify the CylinderModel radius.
    // XXXX ChangeCylinderCommandPtr command_;

    /// Initializes and returns a RangeWidget scaler.
    RangeWidgetPtr InitScaler_(const std::string &name);

    /// Updates both scalers based on the attached CylinderModel.
    void UpdateScalers_();

    void ScalerActivated_(const RangeWidgetPtr &scaler);
    void ScalerChanged_(const RangeWidgetPtr &scaler, bool is_max);

    friend class Parser::Registry;
};
