//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Types.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(Widget);

/// ITouchable is an interface for any class that implements touch with a VR
/// controller.
///
/// \ingroup Widgets
class ITouchable {
  public:
    /// Returns the Widget, if any, that is touched by a touch sphere at the
    /// given position and with the given touch radius. If more than one Widget
    /// is intersected by the sphere, the closest one is returned.
    virtual WidgetPtr GetTouchedWidget(const Point3f &touch_pos,
                                       float radius) const = 0;
};

DECL_SHARED_PTR(ITouchable);
