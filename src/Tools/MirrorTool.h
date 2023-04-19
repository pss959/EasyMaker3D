#pragma once

#include <memory>

#include "Math/Types.h"
#include "Tools/Tool.h"

DECL_SHARED_PTR(ChangeMirrorCommand);
DECL_SHARED_PTR(MirroredModel);
DECL_SHARED_PTR(PlaneWidget);

/// MirrorTool provides interactive clipping of all selected MirroredModel
/// instances. It uses a PlaneWidget to orient and position the mirroring plane.
/// See the ClipTool documentation for details, since this works similarly.
///
/// \ingroup Tools
class MirrorTool : public Tool {
    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual GripGuideType GetGripGuideType() const override {
        return GripGuideType::kBasic;
    }
    virtual void UpdateGripInfo(GripInfo &info) override;

  protected:
    MirrorTool();

    virtual void CreationDone() override;

    virtual bool IsSpecialized() const override { return true; }
    virtual bool CanAttach(const Selection &sel) const override;
    virtual void Attach() override;
    virtual void Detach() override;

  private:
    /// Command used to modify all affected Models.
    ChangeMirrorCommandPtr command_;

    /// Attached MirroredModel (for convenience). This is null when not attached
    /// to the current selection.
    MirroredModelPtr       mm_;

    /// Interactive PlaneWidget used to orient and position the mirroring
    /// plane.
    PlaneWidgetPtr         plane_widget_;

    /// Current mirroring plane in stage coordinates.
    Plane                  stage_plane_;

    /// Plane (stage coordinates) at the start of widget interaction.
    Plane                  start_stage_plane_;

    /// Center of the unclipped model in stage coordinates.
    Point3f                stage_center_;

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

    /// Returns the mirroring plane in stage coordinates from the Plane in the
    /// attached MirroredModel.
    Plane GetStagePlaneFromModel_() const;

    /// Returns the mirroring plane in stage coordinates from the PlaneWidget's
    /// current mirroring plane.
    Plane GetStagePlaneFromWidget_() const;

    /// Returns the current mirroring plane in object coordinates based on the
    /// PlaneWidget's plane and the MirrorTool's rotation.
    Plane GetObjectPlane_() const;

    /// Converts the given Plane from object to stage coordinates.
    Plane ObjectToStagePlane_(const Plane &object_plane) const;

    /// Converts the given Plane from stage to object coordinates.
    Plane StageToObjectPlane_(const Plane &stage_plane) const;

    /// Updates the PlaneWidget to match the current mirroring plane.
    void UpdatePlaneWidgetPlane_();

    friend class Parser::Registry;
};
