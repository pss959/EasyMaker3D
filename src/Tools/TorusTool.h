#pragma once

#include <string>

#include "Commands/ChangeTorusCommand.h"
#include "Feedback/LinearFeedback.h"
#include "Models/TorusModel.h"
#include "Tools/Tool.h"
#include "Widgets/ScaleWidget.h"

/// TorusTool is a specialized Tool that is used to edit the inner and outer
/// radii of a TorusModel.
///
/// \ingroup Tools
class TorusTool : public Tool {
  public:
    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual GripGuideType GetGripGuideType() const override {
        return GripGuideType::kBasic;
    }
    virtual void UpdateGripInfo(GripInfo &info) override;

  protected:
    TorusTool();

    virtual void CreationDone() override;

    virtual bool IsSpecialized() const override { return true; }
    virtual bool CanAttach(const Selection &sel) const override;
    virtual void Attach() override;
    virtual void Detach() override;

  private:
    ScaleWidgetPtr    inner_scaler_;
    ScaleWidgetPtr    outer_scaler_;
    LinearFeedbackPtr feedback_;

    /// Changing radius at the start of a drag.
    float start_radius_ = 0;

    /// Outer radius at the start of a drag. This is used to restore the outer
    /// radius if it changes as the result of a change to the inner radius.
    float start_outer_radius_ = 0;

    /// Attached TorusModel.
    TorusModelPtr torus_model_;

    /// Command used to modify the TorusModel radius.
    ChangeTorusCommandPtr command_;

    /// Initializes and returns a ScaleWidget scaler.
    ScaleWidgetPtr InitScaler_(const std::string &name, bool is_inner);

    /// Updates both scalers based on the attached TorusModel.
    void UpdateScalers_();

    /// Scales the handles and stick of a ScaleWidget based on the Model size.
    void ScaleScaler_(ScaleWidget &scaler, const Vector3f &model_size);

    void ScalerActivated_(const ScaleWidgetPtr &scaler, bool is_activation);
    void ScalerChanged_(const ScaleWidgetPtr &scaler, bool is_max);

    void UpdateFeedback_(const TorusModel &model,
                         bool is_inner, bool is_snapped);

    friend class Parser::Registry;
};
