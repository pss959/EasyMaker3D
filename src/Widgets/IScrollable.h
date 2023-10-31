#pragma once

#include "Util/Memory.h"

DECL_SHARED_PTR(Widget);

/// IScrollable is an interface for any class that responds to valuator (scroll
/// wheel) events.
///
/// \ingroup Widgets
class IScrollable {
  public:
    /// Returns true if the IScrollable handles a valuator change by the given
    /// amount.
    virtual bool ProcessValuator(float delta) = 0;
};
