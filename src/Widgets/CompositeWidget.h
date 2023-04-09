#pragma once

#include "Base/Memory.h"

DECL_SHARED_PTR(CompositeWidget);

/// CompositeWidget is an abstract base class for widgets that are composed of
/// other types of widgets.
///
/// \ingroup Widgets
class CompositeWidget : public Widget {
  protected:
    CompositeWidget() {}
};
