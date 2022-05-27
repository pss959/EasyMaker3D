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
/// default. Scaling of the StageWidget should then be relative to this; the
/// GetDefaultScale() function returns a value that maintains the scale ratio.
///
/// \ingroup Widgets
class StageWidget : public DiscWidget {
  public:
    /// Returns the ProceduralImage used to create the grid on the Stage.
    SG::ProceduralImagePtr GetGridImage() const;

    /// Sets the radius to use for the StageWidget, defining stage coordinate
    /// scale mapping. If this is never called, the geometric radius of the
    /// StageWidget is used (1-to-1 scale mapping). Note that this also resets
    /// the current scale of the StageWidget to GetDefaultScale().
    void SetStageRadius(float radius);

    /// Returns the default (uniform) scale factors used when the Stage is at
    /// its default size, taking the radius set by SetStageRadius() into
    /// account.
    const Vector3f GetDefaultScale() const { return default_scale_; }

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
    /// Scale factors to use when the StageWidget is at its default size. This
    /// is computed from the size of the geometry and the radius set by
    /// SetStageRadius().
    Vector3f default_scale_{1, 1, 1};

    /// Helper function for target placement.
    void GetTargetPlacement_(const DragInfo &info,
                             Point3f &position, Vector3f &direction);

    friend class Parser::Registry;
};
