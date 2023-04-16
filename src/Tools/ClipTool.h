#pragma once

#include <memory>

#include "Math/Types.h"
#include "Tools/Tool.h"

class ClippedModel;
DECL_SHARED_PTR(ChangeClipCommand);
DECL_SHARED_PTR(PlaneWidget);
DECL_SHARED_PTR(LinearFeedback);

/// ClipTool provides interactive clipping of all selected ClippedModel
/// instances. It uses a PlaneWidget to orient and position the clipping
/// plane. The tool is rotated and translated to align with the primary
/// selection (ClippedModel) and the widget is scaled to surround it. This
/// means that the PlaneWidget and its Plane are defined in the object
/// coordinates of the ClipTool. The Plane is converted to stage coordinates
/// when necessary: for the ChangeClipCommand and for real-time clipping.
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

    /// Interactive PlaneWidget used to orient and position the clipping
    /// plane.
    PlaneWidgetPtr       plane_widget_;

    /// Current clipping plane in object coordinates.
    Plane                object_plane_;

    /// Current clipping plane in stage coordinates.
    Plane                stage_plane_;

    /// Plane (stage coordinates) at the start of widget interaction. This is
    /// used to show translation feedback.
    Plane                start_stage_plane_;

    /// Feedback showing translation distance in local coordinates.
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

    /// Returns the current clipping plane in object coordinates based on the
    /// PlaneWidget's plane and the ClipTool's rotation.
    Plane GetObjectPlane_() const;

    /// Converts the given Plane from object to stage coordinates.
    Plane ObjectToStagePlane_(const Plane &object_plane) const;

    /// Converts the given Plane from stage to object coordinates.
    Plane StageToObjectPlane_(const Plane &stage_plane) const;

    /// Sets the min/max range for the translation slider based on the Model's
    /// mesh extents along the current plane's normal.
    void UpdateTranslationRange_();

    /// Updates feedback for plane translation.
    void UpdateTranslationFeedback_(const Color &color);

    /// Updates the state of the real-time clipping plane implemented in the
    /// Faceted shader.
    void UpdateRealTimeClipPlane_(bool enable);

    /// Returns the primary ClippedModel. // XXXX Nuke this.
    ClippedModel & GetPrimary_() const;

    friend class Parser::Registry;
};
