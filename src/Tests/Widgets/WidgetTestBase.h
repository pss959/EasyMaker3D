#pragma once

#include "Place/DragInfo.h"
#include "Tests/SceneTestBase.h"

class DraggableWidget;

// Base class for testing Widgets.
class WidgetTestBase : public SceneTestBase {
  protected:
    // This class can be used to set up DragInfo instances and apply then for
    // testing derived DraggableWidget classes.  The Apply functions each apply
    // a drag operation to a DraggableWidget: it calls StartDrag() with the
    // first DragInfo, ContinueDrag() with the rest, and then EndDrag().
    class DragTester {
      public:
        DragTester();

        // Each of these sets a value that will be copied into all DragInfo
        // instances.
        void SetLinearPrecision(float p);
        void SetAngularPrecision(float p);
        void SetPathToWidget(const SG::NodePath &path);
        void SetRayDirection(const Vector3f &dir);

        // Applies a mouse drag between two points in local coordinates of the
        // given DraggableWidget. If count_between is non-zero, this interpolates
        // the given number of points for the drag.
        void ApplyMouseDrag(DraggableWidget &dw,
                            const Point3f &p0, const Point3f &p1,
                            size_t count_between = 0);

        // Applies a grip rotation drag operation between two orientations to
        // the given DraggableWidget. If count_between is non-zero, this
        // interpolates the given number of orientations for the drag.
        void ApplyGripRotationDrag(DraggableWidget &dw,
                                   const Vector3f &guide_dir,
                                   const Rotationf &r0, const Rotationf &r1,
                                   size_t count_between = 0);

        // Same as the above ApplyGripRotationDrag(), but includes an
        // intermediate orientation.
        void ApplyGripRotationDrag(DraggableWidget &dw,
                                   const Vector3f &guide_dir,
                                   const Rotationf &r0, const Rotationf &r1,
                                   const Rotationf &r2,
                                   size_t count_between = 0);

      private:
        DragInfo base_info_;  // Base DragInfo for setting values.

        void ApplyDrag_(DraggableWidget &dw, const std::vector<DragInfo> &infos);
    };
};
