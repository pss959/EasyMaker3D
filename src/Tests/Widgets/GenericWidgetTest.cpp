#include "Parser/Exception.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/GenericWidget.h"

class GenericWidgetTest : public SceneTestBase {};

TEST_F(GenericWidgetTest, NotifyDrag) {
    auto gw = CreateObject<GenericWidget>();

    size_t start_count = 0;
    size_t drag_count  = 0;
    gw->GetDragged().AddObserver(
        "key", [&](const DragInfo *, bool is_start){
            if (is_start) {
                EXPECT_EQ(0U, drag_count);  // Start must be first.
                ++start_count;
            }
            ++drag_count;
        });

    DragTester dt(gw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(-2, 0, 0), Point3f(2, 0, 0), 3);

    // Should have notified 6 times: 1 start, 1 end.
    EXPECT_EQ(1U, start_count);
    EXPECT_EQ(6U, drag_count);
}

TEST_F(GenericWidgetTest, NotifyHover) {
    auto gw = CreateObject<GenericWidget>();

    std::vector<Point3f> pts;
    gw->GetHovered().AddObserver("key",
                                 [&](const Point3f &p){ pts.push_back(p); });

    // No hovering for regular drag.
    DragTester dt(gw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(-2, 0, 0), Point3f(2, 0, 0), 3);
    EXPECT_EQ(0U, pts.size());

    // These should notify.
    gw->UpdateHoverPoint(Point3f(1, 2, 3));
    EXPECT_EQ(1U, pts.size());
    EXPECT_EQ(Point3f(1, 2, 3), pts[0]);
    gw->UpdateHoverPoint(Point3f(4, 5, 6));
    EXPECT_EQ(2U, pts.size());
    EXPECT_EQ(Point3f(1, 2, 3), pts[0]);
    EXPECT_EQ(Point3f(4, 5, 6), pts[1]);
}
