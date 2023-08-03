#pragma once

#include "Base/Memory.h"
#include "Place/DragInfo.h"
#include "Tests/SceneTestBase.h"

DECL_SHARED_PTR(DraggableWidget);

// Base class for testing Widgets.
class WidgetTestBase : public SceneTestBase {
  protected:
    // This class can be used to set up DragInfo instances and apply then for
    // testing derived DraggableWidget classes.  The Apply functions each apply
    // a drag operation to a DraggableWidget: it calls StartDrag() with the
    // first DragInfo, ContinueDrag() with the rest, and then EndDrag().
    class DragTester {
      public:
        // The constructor is passed the DraggableWidget that will be tested.
        // This initializes the path_to_widget field in all DragInfo instances
        // to contain just the DraggableWidget.
        explicit DragTester(const DraggableWidgetPtr &dw);

        // Each of these sets a value that will be copied into all DragInfo
        // instances.
        void SetLinearPrecision(float p);
        void SetAngularPrecision(float p);
        void SetPathToWidget(const SG::NodePath &path);
        void SetRayDirection(const Vector3f &dir);

        // Applies a mouse drag between two points in local coordinates of the
        // DraggableWidget. If count_between is non-zero, this interpolates the
        // given number of points for the drag.
        void ApplyMouseDrag(const Point3f &p0, const Point3f &p1,
                            size_t count_between = 0);

        // Applies a grip position drag between two points in local coordinates
        // of the DraggableWidget. If count_between is non-zero, this
        // interpolates the given number of points for the drag.
        void ApplyGripDrag(const Point3f &p0, const Point3f &p1,
                           size_t count_between = 0);

        // Same as ApplyGripDrag(), but uses a touch position drag.
        void ApplyTouchDrag(const Point3f &p0, const Point3f &p1,
                            size_t count_between = 0);

        // Applies a grip rotation drag operation between two orientations to
        // the DraggableWidget. If count_between is non-zero, this interpolates
        // the given number of orientations for the drag.
        void ApplyGripRotationDrag(const Vector3f &guide_dir,
                                   const Rotationf &r0, const Rotationf &r1,
                                   size_t count_between = 0);

        // Same as the above ApplyGripRotationDrag(), but includes an
        // intermediate orientation.
        void ApplyGripRotationDrag(const Vector3f &guide_dir,
                                   const Rotationf &r0, const Rotationf &r1,
                                   const Rotationf &r2,
                                   size_t count_between = 0);

      private:
        DraggableWidgetPtr dw_;
        DragInfo           base_info_;  // Base DragInfo for setting values.

        void ApplyDrag_(const std::vector<DragInfo> &infos);
    };
};
