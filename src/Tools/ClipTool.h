#pragma once

#include <memory>

#include "Math/Types.h"
#include "Tools/Tool.h"

/// ClipTool provides interactive clipping of all selected ClippedModels. It
/// uses a PlaneWidget to orient and position the clipping plane
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

    virtual bool IsSpecialized() const override { return true; }
    virtual bool CanAttach(const Selection &sel) const override;
    virtual void Attach() override;
    virtual void Detach() override;

  private:
    struct Impl_;
    std::unique_ptr<Impl_> impl_;

    friend class Parser::Registry;
};
