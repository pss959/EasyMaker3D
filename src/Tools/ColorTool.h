#pragma once

#include <memory>

#include "Commands/ChangeColorCommand.h"
#include "SG/Node.h"
#include "Tools/Tool.h"
#include "Widgets/GenericWidget.h"

/// ColorTool allows the user to change the color of selected Models.
///
/// \ingroup Tools
class ColorTool : public Tool {
    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual GripGuideType GetGripGuideType() const override {
        return GripGuideType::kBasic;
    }
    virtual void UpdateGripInfo(GripInfo &info) override;

  protected:
    ColorTool();

    virtual void CreationDone() override;

    virtual bool CanAttach(const Selection &sel) const override { return true; }
    virtual void Attach() override;
    virtual void Detach() override;

  private:
    /// GenericWidget used to track drags on the color ring.
    GenericWidgetPtr      widget_;

    /// Node used to mark the current color in the ring.
    SG::NodePtr           marker_;

    /// Central disc Node used to display the current color.
    SG::NodePtr           disc_;

    /// Command used to modify all affected Models.
    ChangeColorCommandPtr command_;

    /// GenericWidget drag callback.
    void Dragged_(const DragInfo *info, bool is_start);

    /// Moves the marker to the current color and updates the central disc
    void UpdateColor_();

    friend class Parser::Registry;
};
