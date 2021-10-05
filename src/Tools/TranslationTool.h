#pragma once

#include <memory>

#include "Tools/GeneralTool.h"

/// TranslationTool allows interactive translation along any of the principal
/// coordinate axes.
///
/// \ingroup Tools
class TranslationTool : public GeneralTool {
  protected:
    TranslationTool();
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
    Vector3f start_stage_min_;
    Vector3f start_stage_pos_;
    Vector3f start_stage_max_;

    void FindParts_();
    void UpdateGeometry_();
    void SliderActivated_(int dim, Widget &widget, bool is_activation);
    void SliderChanged_(int dim, Widget &widget, const float &value);

    friend class Parser::Registry;
};
