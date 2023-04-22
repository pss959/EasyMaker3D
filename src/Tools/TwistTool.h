#pragma once

#include <memory>

#include "Tools/Tool.h"

DECL_SHARED_PTR(DiscWidget);
DECL_SHARED_PTR(Slider2DWidget);
DECL_SHARED_PTR(SphereWidget);

/// TwistTool provides interactive twisting of all selected TwistedModels.
/// It has XXXX.
///
/// \ingroup Tools
class TwistTool : public Tool {
    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual GripGuideType GetGripGuideType() const override {
        return GripGuideType::kNone;
    }

  protected:
    TwistTool() {}

    virtual void CreationDone() override;
    virtual bool IsSpecialized() const override { return true; }
    virtual bool CanAttach(const Selection &sel) const override;
    virtual void Attach() override;
    virtual void Detach() override;

  private:
    DiscWidgetPtr     twister_;
    SphereWidgetPtr   axis_rotator_;
    Slider2DWidgetPtr center_translator_;

    friend class Parser::Registry;
};
