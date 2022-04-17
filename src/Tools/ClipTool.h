#pragma once

#include <memory>

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
    struct Impl_;
    std::unique_ptr<Impl_> impl_;

    void AddPlane_(const Plane &obj_plane);
    Plane GetStagePlane_(const Plane &obj_plane);

    friend class Parser::Registry;
};
