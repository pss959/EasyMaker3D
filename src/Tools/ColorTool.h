//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/ChangeColorCommand.h"
#include "SG/Node.h"
#include "Tools/FloatingTool.h"
#include "Widgets/GenericWidget.h"

/// ColorTool allows the user to change the color of selected Models.
///
/// \ingroup Tools
class ColorTool : public FloatingTool {
  public:
    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
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

    /// Point on the ring where the marker is at the start of a drag.
    Point3f               start_ring_pt_;

    void FindParts_();

    /// GenericWidget click callback.
    void Clicked_(const ClickInfo &info);

    /// GenericWidget drag callback.
    void Dragged_(const DragInfo *info, bool is_start);

    /// Moves the marker to the current color and updates the central disc
    void UpdateColor_();

    /// Returns the color to use for the given point on the ring.
    static Color GetRingColor_(const Point3f &ring_point);

    friend class Parser::Registry;
};
