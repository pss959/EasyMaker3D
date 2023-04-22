#pragma once

#include <memory>

#include "Tools/Tool.h"

DECL_SHARED_PTR(ChangeTwistCommand);
DECL_SHARED_PTR(DiscWidget);
DECL_SHARED_PTR(Slider2DWidget);
DECL_SHARED_PTR(SphereWidget);
namespace SG { DECL_SHARED_PTR(Node); }

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

    /// Redefines this to not reset the Widgets.
    virtual void ReattachToSelection() override;

  private:
    /// Command used to modify all affected Models.
    ChangeTwistCommandPtr command_;

    /// Widget used to change the twist axis direction.
    SphereWidgetPtr       axis_rotator_;

    /// Widget used to change the twist center.
    Slider2DWidgetPtr     center_translator_;

    /// Widget used to change the twist angle.
    DiscWidgetPtr         twister_;

    /// Node used to rotate the translator and twister to match the axis
    /// rotation.
    SG::NodePtr           rotator_;

    void UpdateGeometry_();

    // Widget callbacks.
    void Activate_(bool is_activation);
    void TwistChanged_();

    friend class Parser::Registry;
};
