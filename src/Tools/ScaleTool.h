#pragma once

#include <memory>

#include "Commands/ScaleCommand.h"
#include "Tools/Tool.h"

class Dimensionality;

/// ScaleTool allows interactive uniform and non-uniform scaling along any of
/// the principal coordinate axes and planes.
///
/// \ingroup Tools
class ScaleTool : public Tool {
  public:
    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual void UpdateGripInfo(GripInfo &info) override;

  protected:
    ScaleTool();

    virtual void CreationDone() override;

    virtual bool CanAttach(const Selection &sel) const override { return true; }
    virtual void Attach() override;
    virtual void Detach() override;

  private:
    struct Scaler_;
    struct Parts_;
    std::unique_ptr<Parts_> parts_;

    /// This is used to select a Widget for grip hovering.
    std::vector<Vector3f> scaler_dirs_;

    /// Unscaled Model bounds before dragging starts.
    Bounds model_bounds_;

    /// Scaled size of the Model when dragging starts.
    Vector3f start_model_size_;

    /// Length of the active scaler at the start of a drag.
    float    start_length_;

    /// Command used to modify all affected Models.
    ScaleCommandPtr command_;

    void FindParts_();
    void InitScaler_(size_t index, const Str &name, const Vector3f &vec);
    void UpdateGeometry_();

    void ScalerActivated_(size_t index, bool is_activation);
    void ScalerChanged_(size_t index, bool is_max);

    /// Computes and returns the scale ratios based on the current length of
    /// the indexed scaler. Sets snapped_dims to indicate which dimensions, if
    /// any, were snapped.
    Vector3f ComputeRatios_(size_t index, Dimensionality &snapped_dims);

    /// Returns the ScaleCommand::Mode to use for scaling.
    ScaleCommand::Mode GetMode_(const Scaler_ &scaler) const;

    /// Shows or hides feedback in the given dimension(s).
    void EnableFeedback_(const Dimensionality &dims, bool show);

    /// Updates feedback during a drag to show the current size in the given
    /// dimension(s) in Stage coordinates.
    void UpdateFeedback_(const Vector3f &scaler_vec,
                         const Dimensionality &scaler_dims,
                         const Dimensionality &snapped_dims, bool is_max);

    friend class Parser::Registry;
};
