//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Parser/Exception.h"
#include "Place/DragInfo.h"
#include "Place/EdgeTarget.h"
#include "Place/PointTarget.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Util/Memory.h"
#include "Widgets/DiscWidget.h"
#include "Widgets/EdgeTargetWidget.h"
#include "Widgets/ITargetable.h"
#include "Widgets/PointTargetWidget.h"
#include "Widgets/RadialLayoutWidget.h"

/// \ingroup Tests
class TargetWidgetTest : public SceneTestBase {
  protected:
    /// Derived Widget class that supports receiving target placement, always
    /// returning fixed results for testing.
    class TestTargetableWidget : public Widget, public ITargetable {
      public:
        /// Allows changes to CanTargetBounds().
        void SetCanTargetBounds(bool b) { can_target_bounds_ = b; }
        virtual bool CanTargetBounds() const override {
            return can_target_bounds_;
        }
        virtual void PlacePointTarget(const DragInfo &info,
                                      Point3f &position, Vector3f &direction,
                                      Dimensionality &snapped_dims) override {
            if (info.is_modified_mode) { // On bounds.
                position     = Point3f(0, 2, 3);
                direction    = Vector3f(-1, 0, 0);
                snapped_dims = Dimensionality("XYZ");
            }
            else {  // On surface.
                position     = Point3f(1, 2, 3);
                direction    = Vector3f(-1, 0, 0);
                snapped_dims = Dimensionality("XZ");
            }
        }
        virtual void PlaceEdgeTarget(const DragInfo &info, float current_length,
                                     Point3f &position0,
                                     Point3f &position1) override {
            position0 = Point3f(1, 2, 3);
            position1 = Point3f(4, 5, 6);
        }

      protected:
        TestTargetableWidget() {}

      private:
        bool can_target_bounds_ = false;
        friend class Parser::Registry;
    };
    DECL_SHARED_PTR(TestTargetableWidget);

    TargetWidgetTest() {
        Parser::Registry::AddType<TestTargetableWidget>("TestTargetableWidget");
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
    SetParseTypeName("PointTargetWidget");
    TestInvalid("", "Missing target");
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
    auto ttw = CreateObject<TestTargetableWidget>();
    auto ptw = GetPointTargetWidget();

    ptw->SetStageToWorldMatrix(Matrix4f::Identity());

    // Do not hit bounds if the Widget does not support it.
    EXPECT_FALSE(ttw->CanTargetBounds());

    // Set up a parent Node above both the PointTargetWidget and the
    // TestTargetableWidget and add a node with geometry under the
    // TestTargetableWidget. This makes it so that the bounds of the
    // TestTargetableWidget can be intersected.
    auto parent = CreateObject<SG::Node>("Parent");
    auto box = ParseTypedObject<SG::Node>(
        R"(Node "Box" { shapes: [ Box {} ] })");
    EXPECT_NOT_NULL(box);
    parent->AddChild(ptw);
    parent->AddChild(ttw);
    ttw->AddChild(box);

    DragTester dt(ptw, ttw, parent);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 1, 2), Point3f(6, 1.5f, 2));

    EXPECT_EQ(Point3f(1, 2, 3),   ptw->GetPointTarget().GetPosition());
    EXPECT_EQ(Vector3f(-1, 0, 0), ptw->GetPointTarget().GetDirection());

    // Hit bounds. Note that the Ray intersects the Box.
    ttw->SetCanTargetBounds(true);
    EXPECT_TRUE(ttw->CanTargetBounds());

    dt.SetIsModifiedMode(true);
    dt.ApplyMouseDrag(Point3f(1, 0, 0), Point3f(0, 0, 2));

    EXPECT_EQ(Point3f(0, 2, 3),   ptw->GetPointTarget().GetPosition());
    EXPECT_EQ(Vector3f(-1, 0, 0), ptw->GetPointTarget().GetDirection());
}

TEST_F(TargetWidgetTest, PointTargetDragSnapFeedback) {
    auto ttw = CreateObject<TestTargetableWidget>();
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
    SetParseTypeName("EdgeTargetWidget");
    TestInvalid("", "Missing target");
    TestInvalid("target: EdgeTarget { position0: 1 0 0, position1: 1 0 0 }",
                "Length is zero");
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
    auto ttw = CreateObject<TestTargetableWidget>();
    auto etw = GetEdgeTargetWidget();

    etw->SetStageToWorldMatrix(Matrix4f::Identity());

    DragTester dt(etw, ttw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 1, 2), Point3f(6, 1.5f, 2));

    EXPECT_EQ(Point3f(1, 2, 3), etw->GetEdgeTarget().GetPosition0());
    EXPECT_EQ(Point3f(4, 5, 6), etw->GetEdgeTarget().GetPosition1());
}
