#pragma once

#include "Base/Memory.h"
#include "Widgets/DiscWidget.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(StageWidget);
namespace SG { DECL_SHARED_PTR(ProceduralImage); }

/// StageWidget is a derived DiscWidget used for the interactive Stage. It
/// allows a target to be placed.
///
/// The StageWidget geometry has an initial size chosen to look good relative
/// to the room size. However, default stage coordinates (for objects appearing
/// on the stage) are chosen based on the current build volume size (from
/// Settings). The SetStageRadius() function can be used to specify this
/// default.
///
/// \ingroup Widgets
class StageWidget : public DiscWidget {
  public:
    /// Sets the radius to use for the StageWidget, defining stage coordinate
    /// scale mapping. If this is never called, the geometric radius of the
    /// StageWidget is used (1-to-1 scale mapping).
    void SetStageRadius(float radius);

    /// Redefines this to track interactive scaling vs. radius scaling.
    virtual void ApplyScaleChange(float delta) override;

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

    virtual void CreationDone() override;

  private:
    /// Current stage radius.
    float radius_ = 1;

    /// Scale applied to make the radius correct.
    float radius_scale_ = 1;

    /// Scale applied from interactive scaling.
    float interactive_scale_ = 1;

    /// SG::ProceduralImage that draws the grid.
    SG::ProceduralImagePtr grid_image_;

    /// Helper function for target placement.
    void GetTargetPlacement_(const DragInfo &info,
                             Point3f &position, Vector3f &direction);

    friend class Parser::Registry;
};
