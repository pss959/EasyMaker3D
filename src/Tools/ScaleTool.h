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

    /// Command used to modify all affected Models.
    ScaleCommandPtr command_;

    void FindParts_();
    void InitScaler_(size_t index, const std::string &name,
                     const Vector3f &vec);
    void UpdateGeometry_();
    void SliderActivated_(bool is_activation);
    void SliderChanged_(const float &value);

    /// Updates the feedback during a drag showing the current size(s) along
    /// the direction of scaling.
    void UpdateFeedback_();

    friend class Parser::Registry;
};
