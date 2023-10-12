#pragma once

#include <memory>

#include "Math/Spin.h"
#include "Math/Types.h"
#include "Tools/Tool.h"
#include "Widgets/SpinWidget.h"

DECL_SHARED_PTR(ChangeSpinCommand);
DECL_SHARED_PTR(AngularFeedback);
DECL_SHARED_PTR(LinearFeedback);

/// SpinBasedTool is an abstract base class for specialized tools whose
/// interaction involves rotating by some angle about some 3D axis specified as
/// a Spin struct. It uses a SpinWidget to orient and position the axis and to
/// modify the angle. This class assumes the attached Model is derived from
/// ConvertedModel.
///
/// The SpinBasedTool maintains the axis in stage coordinates for use in a
/// ChangeSpinCommand. The axis can be converted to object coordinates when
/// necessary.
///
/// \ingroup Tools
class SpinBasedTool : public Tool {
  public:
    /// All Spin-based tools are specialized.
    virtual bool IsSpecialized() const override { return true; }

    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual void UpdateGripInfo(GripInfo &info) override;

  protected:
    SpinBasedTool();

    virtual void CreationDone() override;
    virtual void Attach() override;
    virtual void Detach() override;

    /// Derived classes must implement this to return the current Spin in
    /// object coordinates of the attached Model.
    virtual Spin GetObjectSpinFromModel() const = 0;

    /// Derived classes must implement this to return a new derived
    /// ChangeSpinCommand that will be used to modify the selected Model(s).
    virtual ChangeSpinCommandPtr CreateChangeSpinCommand() const = 0;

  private:
    /// Command used to modify all affected Models.
    ChangeSpinCommandPtr command_;

    /// Interactive SpinWidget used to orient and position the axis and rotate
    /// around it.
    SpinWidgetPtr        spin_widget_;

    /// Current Spin in stage coordinates.
    Spin                 stage_spin_;

    /// Spin (in stage coordinates) at the start of widget interaction.
    Spin                 start_stage_spin_;

    /// Radius used to set the widget sizes and for feedback placement.
    float                radius_ = 1;

    /// Feedback showing current Spin angle.
    AngularFeedbackPtr   angle_feedback_;

    /// Feedback showing current Spin offset.
    LinearFeedbackPtr    offset_feedback_;

    // Widget callbacks.
    void Activate_(bool is_activation);
    void SpinChanged_(SpinWidget::ChangeType change_type);

    /// Snaps the current Spin axis direction to the PointTarget direction or
    /// principal axis.
    bool SnapAxis_();

    /// Snaps the current Spin center to the PointTarget or model center if
    /// appropriate.
    bool SnapCenter_();

    /// Returns the current Spin in stage coordinates from the attached Model.
    Spin GetStageSpinFromModel_() const;

    /// Returns the Spin in stage coordinates from the SpinWidget's current
    /// values.
    Spin GetStageSpinFromWidget_() const;

    /// Updates the SpinWidget to match the current Spin.
    void UpdateSpinWidget_();

    /// Updates feedback for a change in angle.
    void UpdateAngleFeedback_();

    /// Updates feedback for a change in offset.
    void UpdateOffsetFeedback_();

    /// Returns true if the two given Spin instances differ enough to execute a
    /// command to change them.
    static bool SpinsDiffer_(const Spin &spin0, const Spin &spin1);

    friend class Parser::Registry;
};
