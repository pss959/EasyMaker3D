#pragma once

#include <string>
#include <vector>

#include "Base/Memory.h"
#include "Place/DragInfo.h"
#include "SG/Search.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/String.h"

DECL_SHARED_PTR(CompositeWidget);
DECL_SHARED_PTR(DraggableWidget);
DECL_SHARED_PTR(Widget);

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
        // to contain just the DraggableWidget. If hw is specified, it is used
        // as the Widget in the path in the SG::Hit in all DragInfo instances.
        DragTester(const DraggableWidgetPtr &dw,
                   const WidgetPtr &hw = WidgetPtr());

        // This constructor is passed a CompositeWidget and a vector of
        // sub-widget names used to find the actual DraggableWidget. All but
        // the last name must refer to CompositeWidget sub-widgets, and the
        // last one must be a DraggableWidget sub-widget. The path_to_widget is
        // set to the entire path from the CompositeWidget to the last
        // sub-widget.
        DragTester(const CompositeWidgetPtr &cw,
                   const std::vector<std::string> &names);

        // Each of these sets a value that will be copied into all DragInfo
        // instances.
        void SetIsModifiedMode(bool m);
        void SetLinearPrecision(float p);
        void SetAngularPrecision(float p);
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

        void Init_(const DraggableWidgetPtr &dw);
        void ApplyDrag_(const std::vector<DragInfo> &infos);
    };

    // Some Widget class require some set-up; reading the corresponding data
    // file and adding it to a real Scene is the easiest way to take care of
    // this. This reads the named file and returns the typed Widget from it.
    template <typename T>
    std::shared_ptr<T> GetWidgetOfType(const std::string &file_name,
                                       const std::string &widget_name) {
        const std::string contents = "children: [ <\"" + file_name + "\"> ]";
        const auto input = Util::ReplaceString(
            ReadDataFile("RealScene"), "#<CONTENTS>", contents);
        scene_ = ReadScene(input, true);   // True => sets up grid image.
        auto w = SG::FindTypedNodeInScene<T>(*scene_, widget_name);
        EXPECT_NOT_NULL(w);
        return w;
    }

  private:
    SG::ScenePtr scene_;  // Saves scene used to read StageWidget.
};
