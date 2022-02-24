#pragma once

#include <memory>

#include "Commands/ScaleCommand.h"
#include "Tools/GeneralTool.h"

/// ScaleTool allows interactive uniform and non-uniform scaling along any of
/// the principal coordinate axes and planes.
///
/// \ingroup Tools
class ScaleTool : public GeneralTool {
    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual GripGuideType GetGripGuideType() const override {
        return GripGuideType::kBasic;
    }
    virtual void UpdateGripInfo(GripInfo &info) override;

    virtual void PostSetUpIon() override;

  protected:
    ScaleTool();

    virtual void CreationDone() override;

    virtual void Attach() override;
    virtual void Detach() override;

  private:
    struct Scaler_;
    struct Parts_;
    std::unique_ptr<Parts_> parts_;

    /// Unscaled Model bounds before dragging starts.
    Bounds model_bounds_;

    /// Scaled size of the Model when dragging starts.
    Vector3f start_model_size_;

    /// Length of the active scaler at the start of a drag.
    float    start_length_;

    /// Command used to modify all affected Models.
    ScaleCommandPtr command_;

    void FindParts_();
    void InitScaler_(size_t index, const std::string &name,
                     const Vector3f &vec);
    void UpdateGeometry_();

    void ScalerActivated_(size_t index, bool is_activation);
    void ScalerChanged_(size_t index, bool is_max);

    /// Computes and returns the scale ratios based on the current length of
    /// the indexed scaler. Sets snapped_dims to indicate which dimensions, if
    /// any, were snapped.
    Vector3f ComputeRatios_(size_t index, Dimensionality &snapped_dims);

    friend class Parser::Registry;
};
