#pragma once

#include <memory>

#include "Widgets/DiscWidget.h"

namespace Parser { class Registry; }

/// StageWidget is a derived DiscWidget used for the interactive Stage. It
/// allows a target to be placed.
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
    virtual void PlacePointTarget(const SG::Hit &hit, bool is_alternate_mode,
                                  const SG::NodePath &stage_path,
                                  Point3f &position, Vector3f &direction,
                                  Dimensionality &snapped_dims) override;
  protected:
    StageWidget() {}

  private:
    /// Helper function for target placement.
    void GetTargetPlacement_(const SG::Hit &hit, const SG::NodePath &stage_path,
                             Point3f &position, Vector3f &direction);

    friend class Parser::Registry;
};

typedef std::shared_ptr<StageWidget> StageWidgetPtr;
