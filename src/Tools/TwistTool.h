#pragma once

#include <memory>

#include "Math/Twist.h"
#include "Math/Types.h"
#include "Tools/Tool.h"

class Widget;
DECL_SHARED_PTR(AngularFeedback);
DECL_SHARED_PTR(ChangeTwistCommand);
DECL_SHARED_PTR(WheelWidget);
DECL_SHARED_PTR(Slider2DWidget);
DECL_SHARED_PTR(SphereWidget);
namespace SG { DECL_SHARED_PTR(Node); }

/// TwistTool provides interactive twisting of all selected TwistedModels.
/// It uses a DiscWidget to modify the twist angle, a SphereWidget to modify
/// the twist axis direction, and a Slider2DWidget to modify the twist center.
///
/// \ingroup Tools
class TwistTool : public Tool {
    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual void UpdateGripInfo(GripInfo &info) override;

  protected:
    TwistTool() {}

    virtual void CreationDone() override;
    virtual bool IsSpecialized() const override { return true; }
    virtual bool CanAttach(const Selection &sel) const override;
    virtual void Attach() override;
    virtual void Detach() override;

  private:
    /// Command used to modify all affected Models.
    ChangeTwistCommandPtr command_;

    /// Widget used to rotate the twist axis direction.
    SphereWidgetPtr       rotator_;

    /// Widget used to translate the twist center.
    Slider2DWidgetPtr     translator_;

    /// Widget used to change the twist angle.
    WheelWidgetPtr    twister_;

    /// Node used to apply current rotation to the #translator_ and axis.
    SG::NodePtr           axis_rotator_;

    /// Node used to represent the axis; scaled in the code.
    SG::NodePtr           axis_;

    /// Node representing the cone at the end of the axis.
    SG::NodePtr           cone_;

    /// Node representing the box at the base of the axis.
    SG::NodePtr           base_;

    /// Twist at the start of interaction.
    Twist                 start_twist_;

    /// Current Twist.
    Twist                 twist_;

    /// Feedback showing current twist angle.
    AngularFeedbackPtr    feedback_;

    /// Finds and sets up all of the parts of the tool.
    void SetUpParts_();

    /// Updates the geometry based on the attached TwistedModel.
    void UpdateGeometry_();

    /// Updates all Widgets (without notifying) based on the current Twist.
    void MatchCurrentTwist_();

    // Widget callbacks.
    void Activate_(Widget &widget, bool is_activation);
    void TwistChanged_(Widget &widget);

    /// Snaps the current axis translation to the PointTarget location or an
    /// important point on the attached TwistedModel.
    bool SnapTranslation_();

    /// Snaps the current axis rotation to the PointTarget direction or
    /// principal axis.
    bool SnapRotation_();

    /// Updates the feedback showing the current twist angle.
    void UpdateTwistFeedback_();

    friend class Parser::Registry;
};
