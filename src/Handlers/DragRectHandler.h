#pragma once

#if ENABLE_DEBUG_FEATURES

#include "Handlers/Handler.h"
#include "Math/Types.h"
#include "Util/Memory.h"

struct Event;
DECL_SHARED_PTR(DragRectHandler);
DECL_SHARED_PTR(Frustum);
namespace SG { DECL_SHARED_PTR(Node); }

/// DragRectHandler is used for dragging out a rectangle in the main window,
/// printing the resulting coordinates to stdout. This is used to help take
/// window snapshots for documentation and does not exist in a release build.
///
/// \ingroup Handlers
class DragRectHandler : public Handler {
  public:
    /// Sets the SG::Node containing the rectangle to draw.
    void SetRect(const SG::NodePtr &rect) { rect_ = rect; }

    /// Sets the Frustum used to build rays.
    void SetFrustum(const FrustumPtr &frustum) { frustum_ = frustum; }

    // ------------------------------------------------------------------------
    // Handler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    SG::NodePtr rect_;                 ///< For graphical feedback.
    FrustumPtr  frustum_;              ///< For ray intersections.
    bool        is_dragging_ = false;  ///< True during a drag.
    Point2f     start_point2_;         ///< Start point of drag (2D).
    Point3f     start_point3_;         ///< Start point of drag (3D).

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
