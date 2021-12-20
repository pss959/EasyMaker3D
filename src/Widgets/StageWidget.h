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

    /// Redefines this to place the point target on the surface of the
    /// Model. If is_alternate_mode is true, this places the target on the
    /// bounds. Otherwise places the target on the mesh, checking for snapping
    /// if close to a vertex.
    virtual void PlacePointTarget(const SG::Hit &hit, bool is_alternate_mode,
                                  Point3f &position, Vector3f &direction,
                                  Dimensionality &snapped_dims) override;
  protected:
    StageWidget() {}

  private:
    friend class Parser::Registry;
};

typedef std::shared_ptr<StageWidget> StageWidgetPtr;
