#pragma once

#include <memory>

#include "Commands/RotateCommand.h"
#include "Dimensionality.h"
#include "Tools/GeneralTool.h"

/// RotationTool allows interactive rotation using a DiscWidget around any of
/// the principal coordinate axes using a DiscWidget for each and free rotation
/// using a SphereWidget.
///
/// \ingroup Tools
class RotationTool : public GeneralTool {
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

    /// Creates a RotateCommand and sets it up if not already done.
    void CreateCommandIfNecessary_();

    /// Shows or hides feedback in the given dimension(s).
    void EnableFeedback_(const Dimensionality &dims, bool show);

    /// Updates feedback during a drag to show the current rotation angle
    /// around an axis.
    void UpdateFeedback_(/* XXXX */);

    /// Returns a reasonable outer radius to use for a rotator widget based on
    /// the Model size.
    float GetOuterRadius_() const;

    friend class Parser::Registry;
};
