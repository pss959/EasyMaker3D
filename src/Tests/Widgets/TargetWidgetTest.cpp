#include "Base/Memory.h"
#include "Parser/Exception.h"
#include "Place/DragInfo.h"
#include "Place/EdgeTarget.h"
#include "Place/PointTarget.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/DiscWidget.h"
#include "Widgets/EdgeTargetWidget.h"
#include "Widgets/PointTargetWidget.h"
#include "Widgets/RadialLayoutWidget.h"

/// \ingroup Tests
class TargetWidgetTest : public SceneTestBase {
  protected:
    /// Derived Widget class that supports target placement, always returning
    /// fixed results for testing.
    class TestTargetWidget : public Widget {
      public:
        virtual bool CanReceiveTarget() const override { return true; }
        virtual void PlacePointTarget(const DragInfo &info,
                                      Point3f &position, Vector3f &direction,
                                      Dimensionality &snapped_dims) override {
            position     = Point3f(1, 2, 3);
            direction    = Vector3f(-1, 0, 0);
            snapped_dims = Dimensionality("XZ");
        }
        virtual void PlaceEdgeTarget(const DragInfo &info, float current_length,
                                     Point3f &position0,
                                     Point3f &position1) override {
            position0 = Point3f(1, 2, 3);
            position1 = Point3f(4, 5, 6);
        }

      protected:
        TestTargetWidget() {}

      private:
        friend class Parser::Registry;
    };
    DECL_SHARED_PTR(TestTargetWidget);

    TargetWidgetTest() {
        Parser::Registry::AddType<TestTargetWidget>("TestTargetWidget");
    }

    PointTargetWidgetPtr GetPointTargetWidget() {
        return ReadRealNode<PointTargetWidget>(
            R"(children: [<"nodes/Widgets/PointTargetWidget.emd">])",
            "PointTargetWidget");
    }

    EdgeTargetWidgetPtr GetEdgeTargetWidget() {
        return ReadRealNode<EdgeTargetWidget>(
            R"(children: [<"nodes/Widgets/EdgeTargetWidget.emd">])",
            "EdgeTargetWidget");
    }
};

TEST_F(TargetWidgetTest, PointTargetDefaults) {
    auto ptw = GetPointTargetWidget();

    const auto &pt = ptw->GetPointTarget();
    EXPECT_EQ(Point3f::Zero(),   pt.GetPosition());
    EXPECT_EQ(Vector3f::AxisY(), pt.GetDirection());
    EXPECT_EQ(1,                 pt.GetRadius());
    EXPECT_EQ(CircleArc(),       pt.GetArc());
}

TEST_F(TargetWidgetTest, PointTargetNotValid) {
    TestInvalid("PointTargetWidget {}", "Missing target");
}

TEST_F(TargetWidgetTest, PointTargetSetTarget) {
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

TEST_F(TargetWidgetTest, PointTargetDrag) {
    auto ttw = CreateObject<TestTargetWidget>();
    auto ptw = GetPointTargetWidget();

    ptw->SetStageToWorldMatrix(Matrix4f::Identity());

    DragTester dt(ptw, ttw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 1, 2), Point3f(6, 1.5f, 2));

    EXPECT_EQ(Point3f(1, 2, 3),   ptw->GetPointTarget().GetPosition());
    EXPECT_EQ(Vector3f(-1, 0, 0), ptw->GetPointTarget().GetDirection());
}

TEST_F(TargetWidgetTest, PointTargetDragSnapFeedback) {
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

TEST_F(TargetWidgetTest, PointTargetLayout) {
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

// ----------------------------------------------------------------------------
// PointTargetWidget tests.
// ----------------------------------------------------------------------------

TEST_F(TargetWidgetTest, EdgeTargetDefaults) {
    auto etw = GetEdgeTargetWidget();

    const auto &et = etw->GetEdgeTarget();
    EXPECT_EQ(Point3f::Zero(),  et.GetPosition0());
    EXPECT_EQ(Point3f(0, 4, 0), et.GetPosition1());
}

TEST_F(TargetWidgetTest, EdgeTargetNotValid) {
    TestInvalid("EdgeTargetWidget {}", "Missing target");
    TestInvalid("EdgeTargetWidget { target: EdgeTarget {"
                " position0: 1 0 0, position1: 1 0 0 } }", "Length is zero");
}

TEST_F(TargetWidgetTest, EdgeTargetSetTarget) {
    auto et = CreateObject<EdgeTarget>();
    et->SetPositions(Point3f(1, 2, 3), Point3f(4, 5, 6));

    auto etw = GetEdgeTargetWidget();
    etw->SetEdgeTarget(*et);

    const auto &wet = etw->GetEdgeTarget();
    EXPECT_EQ(et->GetPosition0(), wet.GetPosition0());
    EXPECT_EQ(et->GetPosition1(), wet.GetPosition1());
}

TEST_F(TargetWidgetTest, EdgeTargetDrag) {
    auto ttw = CreateObject<TestTargetWidget>();
    auto etw = GetEdgeTargetWidget();

    etw->SetStageToWorldMatrix(Matrix4f::Identity());

    DragTester dt(etw, ttw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 1, 2), Point3f(6, 1.5f, 2));

    EXPECT_EQ(Point3f(1, 2, 3), etw->GetEdgeTarget().GetPosition0());
    EXPECT_EQ(Point3f(4, 5, 6), etw->GetEdgeTarget().GetPosition1());
}
