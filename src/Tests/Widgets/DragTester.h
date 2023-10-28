#pragma once

#include <string>
#include <vector>

#include "Math/Types.h"
#include "Place/DragInfo.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(CompositeWidget);
DECL_SHARED_PTR(DraggableWidget);
DECL_SHARED_PTR(Widget);

/// This class can be used to set up DragInfo instances and apply then for
/// testing derived DraggableWidget classes. The Apply functions each apply a
/// drag operation to a DraggableWidget: it calls StartDrag() with the first
/// DragInfo, ContinueDrag() with the rest, and then EndDrag().
/// \ingroup Tests
class DragTester {
  public:
    /// The constructor is passed the DraggableWidget that will be tested.
    /// This initializes the path_to_widget field in all DragInfo instances to
    /// contain just the DraggableWidget. If \p part is specified, it is used
    /// as the hit part in the path in the SG::Hit in all DragInfo instances.
    DragTester(const DraggableWidgetPtr &dw,
               const SG::NodePtr &part = SG::NodePtr());

    /// This constructor is passed a CompositeWidget and a vector of sub-widget
    /// names used to find the actual DraggableWidget. All but the last name
    /// must refer to CompositeWidget sub-widgets, and the last one must be a
    /// DraggableWidget sub-widget. The path_to_widget is set to the entire
    /// path from the CompositeWidget to the last sub-widget.
    DragTester(const CompositeWidgetPtr &cw, const StrVec &names);

    /// \name Set-up functions.
    /// Each of these sets a value that will be copied into all DragInfo
    /// instances.
    ///@{
    void SetIsModifiedMode(bool m);
    void SetLinearPrecision(float p);
    void SetAngularPrecision(float p);
    void SetRayDirection(const Vector3f &dir);
    ///@}

    /// Applies a mouse drag between two points in local coordinates of the
    /// DraggableWidget. If count_between is non-zero, this interpolates the
    /// given number of points for the drag. If \p finish_off_widget is true,
    /// the last DragInfo will have an empty path.
    void ApplyMouseDrag(const Point3f &p0, const Point3f &p1,
                        size_t count_between = 0,
                        bool finish_off_widget = false) {
        ApplyMultiMouseDrag(std::vector<Point3f>{ p0, p1 }, count_between,
                            finish_off_widget);
    }

    /// Same as ApplyMouseDrag() but allows for multiple points to be
    /// specified. If count_between is non-zero, this interpolates the given
    /// number of points for each pair of points in the vector. If \p
    /// finish_off_widget is true, the last DragInfo will have an empty path.
    void ApplyMultiMouseDrag(const std::vector<Point3f> &pts,
                             size_t count_between = 0,
                             bool finish_off_widget = false);

    /// Applies a grip position drag between two points in local coordinates of
    /// the DraggableWidget. If count_between is non-zero, this interpolates
    /// the given number of points for the drag.
    void ApplyGripDrag(const Point3f &p0, const Point3f &p1,
                       size_t count_between = 0);

    /// Same as ApplyGripDrag(), but uses a touch position drag.
    void ApplyTouchDrag(const Point3f &p0, const Point3f &p1,
                        size_t count_between = 0);

    /// Applies a grip rotation drag operation between two orientations to the
    /// DraggableWidget. If count_between is non-zero, this interpolates the
    /// given number of orientations for the drag.
    void ApplyGripRotationDrag(const Vector3f &guide_dir,
                               const Rotationf &r0, const Rotationf &r1,
                               size_t count_between = 0);

    /// Same as the above ApplyGripRotationDrag(), but includes an intermediate
    /// orientation.
    void ApplyGripRotationDrag(const Vector3f &guide_dir,
                               const Rotationf &r0, const Rotationf &r1,
                               const Rotationf &r2,
                               size_t count_between = 0);

  private:
    DraggableWidgetPtr dw_;
    DragInfo           base_info_;  /// Base DragInfo for setting values.

    void Init_(const DraggableWidgetPtr &dw);
    void ApplyDrag_(const std::vector<DragInfo> &infos);
};
