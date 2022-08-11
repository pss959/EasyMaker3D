#pragma once

#include <memory>

#include "Base/Dimensionality.h"
#include "Commands/RotateCommand.h"
#include "Math/Types.h"
#include "Tools/Tool.h"

/// RotationTool allows interactive rotation using a DiscWidget around any of
/// the principal coordinate axes using a DiscWidget for each and free rotation
/// using a SphereWidget.
///
/// \ingroup Tools
class RotationTool : public Tool {
    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual GripGuideType GetGripGuideType() const override {
        return GripGuideType::kRotation;
    }
    virtual void UpdateGripInfo(GripInfo &info) override;

  protected:
    RotationTool();

    virtual void CreationDone() override;

    virtual bool CanAttach(const Selection &sel) const override { return true; }
    virtual void Attach() override;
    virtual void Detach() override;

  private:
    struct Rotationr_;
    struct Parts_;
    std::unique_ptr<Parts_> parts_;

    /// Size of Model in stage coordinates.
    Vector3f model_size_;

    /// Rotation at the start of the drag.
    Rotationf start_rot_;

    /// Set to true for in-place rotation (alternate mode).
    bool is_in_place_ = false;

    /// Command used to modify all affected Models.
    RotateCommandPtr command_;

    void FindParts_();
    void UpdateGeometry_();

    void AxisRotatorActivated_(int dim, bool is_activation);
    void AxisRotatorChanged_(int dim, const Anglef &angle);

    void FreeRotatorActivated_(bool is_activation);
    void FreeRotatorChanged_(const Rotationf &rot);

    /// Used when activating any Widget. If dim < 0, use free rotation.
    void Activate_(const Dimensionality &dims, int dim);

    /// Used when deactivating any Widget.
    void Deactivate_(const Dimensionality &dims);

    /// Starts an active rotation: creates a RotateCommand and sets it up, and
    /// also enables feedback for the given dimension(s).
    void StartRotation_(const Dimensionality &dims);

    /// Tries snapping the given rotation to the current point target direction
    /// if the target is active. If any of the coordinate axes (ignoring the
    /// one for dim, if not negative) is close enough to the target direction,
    /// this modifies rot to include the snapping rotation to align the axis
    /// and returns that dimension. Otherwise, it returns -1.
    int SnapRotation_(int dim, Rotationf &rot);

    /// Shows or hides feedback in the given dimension(s).
    void EnableFeedback_(const Dimensionality &dims, bool show);

    /// Updates feedback during a drag to show the current rotation angle
    /// around an axis. The text_up_offset is used when multiple dimensions are
    /// shown to keep the text displays from overlapping.
    void UpdateFeedback_(int dim, const Anglef &angle, bool is_snapped,
                         float text_up_offset);

    /// Returns a reasonable outer radius to use for a rotator widget based on
    /// the Model size.
    float GetOuterRadius_() const;

    /// Composes two rotations in the proper order so r1 has more local effect
    /// than r0, taking a axis-aligned flag into account: when axis-aligned
    /// rotation is in effect, the order is reversed.
    Rotationf ComposeRotations_(const Rotationf &r0, const Rotationf &r1,
                                bool is_axis_aligned);

    friend class Parser::Registry;
};
