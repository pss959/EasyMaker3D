#pragma once

#if ENABLE_DEBUG_FEATURES

#include "Base/Memory.h"
#include "Handlers/Handler.h"

struct Event;
DECL_SHARED_PTR(DragRectHandler);

/// DragRectHandler is used for dragging out a rectangle in the main window,
/// printing the resulting coordinates to stdout. This is used to help take
/// window snapshots for documentation and does not exist in a release build.
///
/// \ingroup Handlers
class DragRectHandler : public Handler {
  public:
    // ------------------------------------------------------------------------
    // Handler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    bool is_dragging_ = false;  ///< True while a drag is in progress.

    /// Returns true if the given Event starts a drag.
    static bool IsDragStart_(const Event &event);
    /// Returns true if the given Event ends a drag.
    static bool IsDragEnd_(const Event &event);
};

#endif
