#include "Parser/Exception.h"
#include "Place/DragInfo.h"
#include "Place/PointTarget.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/TargetWidgetTestBase.h"
#include "Widgets/DiscWidget.h"
#include "Widgets/PointTargetWidget.h"
#include "Widgets/RadialLayoutWidget.h"

class PointTargetWidgetTest : public TargetWidgetTestBase {
  protected:
    PointTargetWidgetPtr GetPointTargetWidget() {
        return GetWidgetOfType<PointTargetWidget>("nodes/PointTarget.emd",
                                                  "PointTarget");
    }
};

TEST_F(PointTargetWidgetTest, Defaults) {
    auto ptw = GetPointTargetWidget();

    const auto &pt = ptw->GetPointTarget();
    EXPECT_EQ(Point3f::Zero(),   pt.GetPosition());
    EXPECT_EQ(Vector3f::AxisY(), pt.GetDirection());
    EXPECT_EQ(1,                 pt.GetRadius());
    EXPECT_EQ(CircleArc(),       pt.GetArc());
}

TEST_F(PointTargetWidgetTest, NotValid) {
    TEST_THROW(ParseObject<PointTargetWidget>("PointTargetWidget {}"),
               Parser::Exception, "Missing target");
}

TEST_F(PointTargetWidgetTest, SetTarget) {
    auto pt = CreateObject<PointTarget>();
    pt->SetPosition(Point3f(1, 2, 3));
    pt->SetDirection(Vector3f(0, 0, -4));
    pt->SetRadius(3.5f);
    pt->SetArc(CircleArc(Anglef::FromDegrees(20), Anglef::FromDegrees(-100)));

    auto ptw = GetPointTargetWidget();
    ptw->SetPointTarget(*pt);

    const auto &wpt = ptw->GetPointTarget();
    EXPECT_EQ(pt->GetPosition(),  wpt.GetPosition());
    EXPECT_EQ(pt->GetDirection(), wpt.GetDirection());
    EXPECT_EQ(pt->GetRadius(),    wpt.GetRadius());
    EXPECT_EQ(pt->GetArc(),       wpt.GetArc());
}

TEST_F(PointTargetWidgetTest, Drag) {
    auto ttw = CreateObject<TestTargetWidget>();
    auto ptw = GetPointTargetWidget();

    ptw->SetStageToWorldMatrix(Matrix4f::Identity());

    DragTester dt(ptw, ttw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 1, 2), Point3f(6, 1.5f, 2));

    EXPECT_EQ(Point3f(1, 2, 3),   ptw->GetPointTarget().GetPosition());
    EXPECT_EQ(Vector3f(-1, 0, 0), ptw->GetPointTarget().GetDirection());
}

TEST_F(PointTargetWidgetTest, DragSnapFeedback) {
    auto ttw = CreateObject<TestTargetWidget>();
    auto ptw = GetPointTargetWidget();

    ptw->SetSnapFeedbackPoint(Point3f(1, 1, 1));
    ptw->ShowSnapFeedback(true);

    DragTester dt(ptw, ttw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 1, 2), Point3f(6, 1.5f, 2));

    EXPECT_EQ(Point3f(1, 2, 3),   ptw->GetPointTarget().GetPosition());
    EXPECT_EQ(Vector3f(-1, 0, 0), ptw->GetPointTarget().GetDirection());

    // Should have no effect.
    ptw->ShowSnapFeedback(true);

    ptw->ShowSnapFeedback(false);
}

TEST_F(PointTargetWidgetTest, Layout) {
    auto ptw = GetPointTargetWidget();

    // Access the DiscWidget ring in the RadialLayoutWidget in the
    // PointTargetWidget and drag it to change the ring size.
    auto dw = SG::FindTypedNodeUnderNode<DiscWidget>(
        *SG::FindTypedNodeUnderNode<RadialLayoutWidget>(
            *ptw, "RadialLayoutWidget"), "Ring");

    DragTester dt(dw);
    dt.SetRayDirection(-Vector3f::AxisY());
    dt.ApplyMouseDrag(Point3f(0, 0, 2), Point3f(0, 0, 4));
}
