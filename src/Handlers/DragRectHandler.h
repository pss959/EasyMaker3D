#pragma once

#if ENABLE_DEBUG_FEATURES

#include "Base/Memory.h"
#include "Handlers/Handler.h"
#include "Math/Types.h"

struct Event;
DECL_SHARED_PTR(DragRectHandler);
DECL_SHARED_PTR(SceneContext);

/// DragRectHandler is used for dragging out a rectangle in the main window,
/// printing the resulting coordinates to stdout. This is used to help take
/// window snapshots for documentation and does not exist in a release build.
///
/// \ingroup Handlers
class DragRectHandler : public Handler {
  public:
    /// Sets the SceneContext to interact with.
    void SetSceneContext(const SceneContextPtr &context) {
        scene_context_ = context;
    }

    // ------------------------------------------------------------------------
    // Handler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    SceneContextPtr scene_context_;        ///< For graphical feedback.
    bool            is_dragging_ = false;  ///< True during a drag.
    Point2f         start_point2_;         ///< Start point of drag (2D).
    Point3f         start_point3_;         ///< Start point of drag (3D).

    /// Returns true if the given Event starts a drag.
    static bool IsDragStart_(const Event &event);
    /// Returns true if the given Event ends a drag.
    static bool IsDragEnd_(const Event &event);

    /// Returns the 3D rectangle point corresponding to the 2D mouse position
    /// in the given Event.
    Point3f GetPoint3_(const Event &event) const;

    /// Prints the resulting rectangle.
    static void PrintRect_(const Point2f &p0, const Point2f &p1);
};

#endif
