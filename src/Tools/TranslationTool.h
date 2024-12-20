//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <memory>

#include "Commands/TranslateCommand.h"
#include "Tools/Tool.h"

/// TranslationTool allows interactive translation along any of the principal
/// coordinate axes.
///
/// \ingroup Tools
class TranslationTool : public Tool {
  public:
    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual void UpdateGripInfo(GripInfo &info) override;

  protected:
    TranslationTool();

    virtual void CreationDone() override;

    virtual bool CanAttach(const Selection &sel) const override { return true; }
    virtual void Attach() override;
    virtual void Detach() override;

  private:
    struct Parts_;
    std::unique_ptr<Parts_> parts_;

    /// Scaled size of the Model to use for surrounding it and snapping.
    Vector3f model_size_;

    /// Starting value of a slider that is being dragged.
    float start_value_;

    /// These store the three points in stage coordinates used for potential
    /// snapping operations for motion in a given dimension. These are the
    /// intersection with the motion vector in that dimension with the min,
    /// center, and max of bounds at the start of a drag operation.
    Point3f start_stage_min_;
    Point3f start_stage_pos_;
    Point3f start_stage_max_;

    /// Command used to modify all affected Models.
    TranslateCommandPtr command_;

    void FindParts_();
    void UpdateGeometry_();
    void SliderActivated_(int dim, bool is_activation);
    void SliderChanged_(int dim, const float &value);

    /// Updates the feedback during a drag showing the amount of relative motion
    /// being applied.
    void UpdateFeedback_(int dim, const Vector3f &motion, bool is_snapped);

    /// Returns the current slider value in the given dimension.
    float GetSliderValue_(int dim) const;

    friend class Parser::Registry;
};
