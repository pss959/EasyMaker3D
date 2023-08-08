#include "Parser/Exception.h"
#include "Place/DragInfo.h"
#include "Place/EdgeTarget.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Tests/Widgets/TargetSceneTestBase.h"
#include "Widgets/EdgeTargetWidget.h"

class EdgeTargetWidgetTest : public TargetSceneTestBase {
  protected:
    EdgeTargetWidgetPtr GetEdgeTargetWidget() {
        return GetWidgetOfType<EdgeTargetWidget>("nodes/EdgeTarget.emd",
                                                 "EdgeTarget");
    }
};

TEST_F(EdgeTargetWidgetTest, Defaults) {
    auto etw = GetEdgeTargetWidget();

    const auto &et = etw->GetEdgeTarget();
    EXPECT_EQ(Point3f::Zero(),  et.GetPosition0());
    EXPECT_EQ(Point3f(0, 4, 0), et.GetPosition1());
}

TEST_F(EdgeTargetWidgetTest, NotValid) {
    TEST_THROW(ParseObject<EdgeTargetWidget>("EdgeTargetWidget {}"),
               Parser::Exception, "Missing target");

    const std::string input = R"(
EdgeTargetWidget {
  target: EdgeTarget {
    position0:   1 0 0,
    position1:   1 0 0,
  }
})";
    TEST_THROW(ParseObject<EdgeTargetWidget>(input),
               Parser::Exception, "Length is zero");
}

TEST_F(EdgeTargetWidgetTest, SetTarget) {
    auto et = CreateObject<EdgeTarget>();
    et->SetPositions(Point3f(1, 2, 3), Point3f(4, 5, 6));

    auto etw = GetEdgeTargetWidget();
    etw->SetEdgeTarget(*et);

    const auto &wet = etw->GetEdgeTarget();
    EXPECT_EQ(et->GetPosition0(), wet.GetPosition0());
    EXPECT_EQ(et->GetPosition1(), wet.GetPosition1());
}

TEST_F(EdgeTargetWidgetTest, Drag) {
    auto ttw = CreateObject<TestTargetWidget>();
    auto etw = GetEdgeTargetWidget();

    etw->SetStageToWorldMatrix(Matrix4f::Identity());

    DragTester dt(etw, ttw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 1, 2), Point3f(6, 1.5f, 2));

    EXPECT_EQ(Point3f(1, 2, 3), etw->GetEdgeTarget().GetPosition0());
    EXPECT_EQ(Point3f(4, 5, 6), etw->GetEdgeTarget().GetPosition1());
}
