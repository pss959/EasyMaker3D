#pragma once

#include <memory>

#include "Math/Types.h"
#include "Tools/Tool.h"

DECL_SHARED_PTR(ChangeClipCommand);
DECL_SHARED_PTR(ClippedModel);
DECL_SHARED_PTR(PlaneWidget);
DECL_SHARED_PTR(LinearFeedback);

/// ClipTool provides interactive clipping of all selected ClippedModel
/// instances. It uses a PlaneWidget to orient and position the clipping
/// plane.

/// Some notes on transformations:
///   - The ClipTool is rotated to align with the primary ClippedModel (even if
///     is_axis_aligned is true).
///   - The PlaneWidget is explicitly sized to surround the ClippedModel. Any
///     scale in the ClippedModel does \em not apply to the PlaneWidget's
///     plane.
///   - The ClipTool is translated so that it is centered on the ClippedModel's
///     operand mesh (i.e., the unclipped mesh) using the center offset in the
///     ClippedModel. This offset has to be dealt with when converting to and
///     from stage coordinates.
///
/// The ClipTool maintains the clipping plane only in stage coordinates; this
/// plane is used for the ChangeClipCommand and for real-time clipping. The
/// clipping plane is converted to other coordinates when necessary.
///
/// \ingroup Tools
class ClipTool : public Tool {
    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual GripGuideType GetGripGuideType() const override {
        return GripGuideType::kBasic;
    }
    virtual void UpdateGripInfo(GripInfo &info) override;

  protected:
    ClipTool();

    virtual void CreationDone() override;

    virtual bool IsSpecialized() const override { return true; }
    virtual bool CanAttach(const Selection &sel) const override;
    virtual void Attach() override;
    virtual void Detach() override;

  private:
    /// Command used to modify all affected Models.
    ChangeClipCommandPtr command_;

    /// Attached ClippedModel (for convenience). This is null when not attached
    /// to the current selection.
    ClippedModelPtr      cm_;

    /// Interactive PlaneWidget used to orient and position the clipping
    /// plane.
    PlaneWidgetPtr       plane_widget_;

    /// Current clipping plane in stage coordinates.
    Plane                stage_plane_;

    /// Plane (stage coordinates) at the start of widget interaction. This is
    /// used to show translation feedback.
    Plane                start_stage_plane_;

    /// Center of the unclipped model in stage coordinates.
    Point3f              stage_center_;

    /// Feedback showing ClippedModel size in the direction of the plane normal.
    LinearFeedbackPtr    feedback_;

    // Widget callbacks.
    void Activate_(bool is_activation);
    void PlaneChanged_(bool is_rotation);

    /// If the current PlaneWidget rotation should be snapped to the
    /// PointTarget or principal axis, this returns true. It sets \p
    /// snapped_dim to the axis dimension if appropriate; otherwise it sets it
    /// to -1.
    bool SnapRotation_(int &snapped_dim);

    /// If the current PlaneWidget translation should be snapped to the
    /// PointTarget or origin, this returns true.
    bool SnapTranslation_();

    /// Returns the clipping plane in stage coordinates from the Plane in the
    /// attached ClippedModel.
    Plane GetStagePlaneFromModel_() const;

    /// Returns the clipping plane in stage coordinates from the PlaneWidget's
    /// current clipping plane.
    Plane GetStagePlaneFromWidget_() const;

    /// Returns the current clipping plane in object coordinates based on the
    /// PlaneWidget's plane and the ClipTool's rotation.
    Plane GetObjectPlane_() const;

    /// Converts the given Plane from object to stage coordinates.
    Plane ObjectToStagePlane_(const Plane &object_plane) const;

    /// Converts the given Plane from stage to object coordinates.
    Plane StageToObjectPlane_(const Plane &stage_plane) const;

    /// Updates the PlaneWidget to match the current clipping plane.
    void UpdatePlaneWidgetPlane_();

    /// Sets the min/max range for the translation slider based on the Model's
    /// mesh extents along the current plane's normal.
    void UpdateTranslationRange_();

    /// Updates feedback for plane translation.
    void UpdateTranslationFeedback_(const Color &color);

    /// Updates the state of the real-time clipping plane implemented in the
    /// Faceted shader.
    void UpdateRealTimeClipPlane_(bool enable);

    friend class Parser::Registry;
};
