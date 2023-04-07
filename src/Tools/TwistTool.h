#pragma once

#include <memory>

#include "Math/Types.h"
#include "Tools/Tool.h"
#include "Widgets/PushButtonWidget.h"

/// TwistTool provides interactive twisting of all selected TwistedModels.
/// It has 3 planes aligned with the coordinate axes that can be twisted
/// across. Each plane is a PushButtonWidget.
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

    virtual bool IsSpecialized() const override { return true; }
    virtual bool CanAttach(const Selection &sel) const override;
    virtual void Attach() override;
    virtual void Detach() override;

  private:
    /// Planes in all 3 dimensions.
    PushButtonWidgetPtr planes_[3]{nullptr, nullptr, nullptr};

    /// Plane click callback.
    void PlaneClicked_(int dim);

    friend class Parser::Registry;
};
