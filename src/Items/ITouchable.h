#pragma once

#include "Math/Types.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(Widget);

/// ITouchable is an interface class that can be used by a class that can
/// implement touch activation in VR.
///
/// \ingroup Items
class ITouchable {
  public:
    /// Returns the Widget, if any, that is touched by a touch sphere at the
    /// given position and with the given touch radius. If more than one Widget
    /// is intersected by the sphere, the closest one is returned.
    virtual WidgetPtr GetTouchedWidget(const Point3f &touch_pos,
                                       float radius) const = 0;
};
