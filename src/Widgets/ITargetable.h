#pragma once

#include "Math/Dimensionality.h"
#include "Math/Types.h"
#include "Util/Memory.h"

struct DragInfo;

/// ITargetable is an interface for any class that can receive a Target,
/// meaning that a PointTarget or EdgeTarget can be placed or dragged over it.
///
/// \ingroup Widgets
class ITargetable {
  public:
    /// Returns true if the ITargetable distinguishes between placing Targets
    /// on the bounds vs. the surface.
    virtual bool CanTargetBounds() const = 0;

    /// Places a PointTarget on the ITargetable according to the pointer drag
    /// information in the given DragInfo. It should set \p position and \p
    /// direction to place the PointTarget (in stage coordinates) and set \p
    /// snapped_dims to the dimensions (if any) in which the position was
    /// snapped to a feature.
    virtual void PlacePointTarget(const DragInfo &info,
                                  Point3f &position, Vector3f &direction,
                                  Dimensionality &snapped_dims) = 0;

    /// Places an EdgeTarget on the ITargetable according to the pointer drag
    /// information in the given DragInfo. It should set \p position0 and \p
    /// position1 to the new endpoints of the EdgeTarget (in stage
    /// coordinates). The current edge length is passed in for reference.
    ///
    /// The base class defines this to assert.
    virtual void PlaceEdgeTarget(const DragInfo &info, float current_length,
                                 Point3f &position0, Point3f &position1) = 0;
};

DECL_SHARED_PTR(ITargetable);
