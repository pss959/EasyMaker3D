#pragma once

#include "Base/Memory.h"
#include "Widgets/DiscWidget.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(StageWidget);

/// StageWidget is a derived DiscWidget used for the interactive Stage. It
/// allows a target to be placed.
///
/// \ingroup Widgets
class StageWidget : public DiscWidget {
  public:
    // ------------------------------------------------------------------------
    // Target Interface.
    // ------------------------------------------------------------------------

    /// The Stage can receive Targets.
    virtual bool CanReceiveTarget() const override { return true; }

    /// Redefines this to place the point target on the Stage, pointing up,
    /// snapping to grid points.
    virtual void PlacePointTarget(const DragInfo &info,
                                  Point3f &position, Vector3f &direction,
                                  Dimensionality &snapped_dims) override;

    /// Redefines this to place the edge target on the Stage, pointing up,
    /// snapping to grid points.
    virtual void PlaceEdgeTarget(const DragInfo &info, float current_length,
                                 Point3f &position0,
                                 Point3f &position1) override;

  protected:
    StageWidget() {}

  private:
    /// Helper function for target placement.
    void GetTargetPlacement_(const DragInfo &info,
                             Point3f &position, Vector3f &direction);

    friend class Parser::Registry;
};
