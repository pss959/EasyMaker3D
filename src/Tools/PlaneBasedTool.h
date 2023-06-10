#pragma once

#include <memory>

#include "Math/Types.h"
#include "Tools/Tool.h"

DECL_SHARED_PTR(ChangePlaneCommand);
DECL_SHARED_PTR(PlaneWidget);
DECL_SHARED_PTR(LinearFeedback);

/// PlaneBasedTool is an abstract base class for specialized tools whose
/// interaction consists of a single plane; it uses a PlaneWidget to orient and
/// position the plane. This class assumes the attached Model is derived from
/// ConvertedModel.
///
/// The PlaneBasedTool maintains the plane only in stage coordinates; this
/// plane is used for the ChangePlaneCommand. The plane can be converted to
/// object coordinates when necessary.
///
/// \ingroup Tools
class PlaneBasedTool : public Tool {
  public:
    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual void UpdateGripInfo(GripInfo &info) override;

  protected:
    PlaneBasedTool();

    virtual void CreationDone() override;
    virtual void Attach() override;
    virtual void Detach() override;

    /// Derived classes must implement this to return the current Plane in
    /// object coordinates of the attached Model.
    virtual Plane GetObjectPlaneFromModel() const = 0;

    /// Derived classes must implement this to return the translation range to
    /// use for the translation widget in the PlaneWidget.
    virtual Range1f GetTranslationRange() const = 0;

    /// Derived classes must implement this to return a new derived
    /// ChangePlaneCommand that will be used to modify the selected Model(s).
    virtual ChangePlaneCommandPtr CreateChangePlaneCommand() const = 0;

    /// Derived classes must implement this to return the base point (in stage
    /// coordinates) for translation feedback.
    virtual Point3f GetTranslationFeedbackBasePoint() const = 0;

    /// Returns the current Plane in object coordinates.
    Plane GetObjectPlane() const;

  private:
    /// Command used to modify all affected Models.
    ChangePlaneCommandPtr command_;

    /// Interactive PlaneWidget used to orient and position the plane.
    PlaneWidgetPtr        plane_widget_;

    /// Current plane in stage coordinates.
    Plane                 stage_plane_;

    /// Plane (stage coordinates) at the start of widget interaction. This is
    /// used to show translation feedback.
    Plane                 start_stage_plane_;

    /// Center of the model in stage coordinates.
    Point3f               stage_center_;

    /// Feedback showing motion in the direction of the plane normal.
    LinearFeedbackPtr     feedback_;

    // Widget callbacks.
    void Activate_(bool is_activation);
    void PlaneChanged_(bool is_rotation);

    /// Snaps the current axis rotation to the PointTarget direction or
    /// principal axis.
    bool SnapRotation_();

    /// Snaps the current PlaneWidget translation to the PointTarget or model
    /// center if appropriate.
    bool SnapTranslation_();

    /// Returns the current plane in stage coordinates from the attached Model.
    Plane GetStagePlaneFromModel_() const;

    /// Returns the plane in stage coordinates from the PlaneWidget's current
    /// plane.
    Plane GetStagePlaneFromWidget_() const;

    /// Updates the PlaneWidget to match the current plane.
    void UpdatePlaneWidgetPlane_();

    /// Sets the min/max range for the translation slider based on the Model's
    /// mesh extents along the current plane's normal.
    void UpdateTranslationRange_();

    /// Updates feedback for plane translation.
    void UpdateTranslationFeedback_(bool is_snapped);

    friend class Parser::Registry;
};
