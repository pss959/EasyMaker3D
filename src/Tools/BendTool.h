#pragma once

#include <memory>

#include "Math/Bend.h"
#include "Math/Types.h"
#include "Tools/Tool.h"

class Widget;
DECL_SHARED_PTR(AngularFeedback);
DECL_SHARED_PTR(AxisWidget);
DECL_SHARED_PTR(ChangeBendCommand);
DECL_SHARED_PTR(DiscWidget);
DECL_SHARED_PTR(HandWheelWidget);
// XXXX
//DECL_SHARED_PTR(Slider2DWidget);
//DECL_SHARED_PTR(SphereWidget);
namespace SG { DECL_SHARED_PTR(Node); }

/// BendTool provides interactive bending of all selected BentModels.  It uses
/// a DiscWidget to modify the bend angle, a SphereWidget to modify the bend
/// axis direction, and a Slider2DWidget to modify the bend center.
///
/// \ingroup Tools
class BendTool : public Tool {
    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual void UpdateGripInfo(GripInfo &info) override;

  protected:
    BendTool() {}

    virtual void CreationDone() override;
    virtual bool IsSpecialized() const override { return true; }
    virtual bool CanAttach(const Selection &sel) const override;
    virtual void Attach() override;
    virtual void Detach() override;

  private:
    /// Command used to modify all affected Models.
    ChangeBendCommandPtr command_;

    /// Widget used to rotate the bend axis direction.
    AxisWidgetPtr         axis_;

    /// Widget used to change the bend angle.
    HandWheelWidgetPtr    bender_;

#if XXXX
    /// Widget used to rotate the bend axis direction.
    SphereWidgetPtr       rotator_;

    /// Widget used to translate the bend center.
    Slider2DWidgetPtr     translator_;

    /// Node used to apply current rotation to the #translator_ and axis.
    SG::NodePtr           axis_rotator_;

    /// Node used to represent the axis; scaled in the code.
    SG::NodePtr           axis_;

    /// Node representing the cone at the -Z end of the axis.
    SG::NodePtr           cone0_;

    /// Node representing the cone at the +Z end of the axis.
    SG::NodePtr           cone1_;
#endif

    /// Bend at the start of interaction.
    Bend                  start_bend_;

    /// Current Bend.
    Bend                  bend_;

    /// Feedback showing current bend angle.
    AngularFeedbackPtr    feedback_;

    /// Finds and sets up all of the parts of the tool.
    void SetUpParts_();

    /// Updates the geometry based on the attached BentModel.
    void UpdateGeometry_();

    /// Updates all Widgets (without notifying) based on the current Bend.
    void MatchCurrentBend_();

    // Widget callbacks.
    void Activate_(Widget &widget, bool is_activation);
    void BendChanged_(Widget &widget);

    /// Snaps the current axis translation to the PointTarget location or an
    /// important point on the attached BentModel.
    bool SnapTranslation_();

    /// Snaps the current axis rotation to the PointTarget direction or
    /// principal axis.
    bool SnapRotation_();

    /// Updates the feedback showing the current bend angle.
    void UpdateBendFeedback_();

    friend class Parser::Registry;
};
