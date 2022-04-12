#pragma once

#include <memory>

#include "Commands/ChangeClipCommand.h"
#include "Math/Types.h"
#include "Tools/Tool.h"

/// ClipTool provides interactive clipping of all selected ClippedModels. It
/// has a SphereWidget used to rotate the clipping plane, a Slider1DWidget
/// (arrow) used to translate the plane along its normal, and a
/// PushButtonWidget (plane) used to apply the clipping plane to the Models.
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
    struct Parts_;
    std::unique_ptr<Parts_> parts_;

    void FindParts_();
    void UpdateGeometry_();
    void MatchPlane_(const Plane &plane);
    void RotatorActivated_(bool is_activation);
    void Rotate_();
    void UpdateRealTimeClipPlane_(bool enable);

    friend class Parser::Registry;
};
