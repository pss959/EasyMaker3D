#include "Commands/ChangeEdgeTargetCommand.h"
#include "Commands/ChangePointTargetCommand.h"
#include "Commands/CommandList.h"
#include "Managers/CommandManager.h"
#include "Managers/TargetManager.h"
#include "SG/Search.h"
#include "Place/ClickInfo.h"
#include "Place/EdgeTarget.h"
#include "Place/PointTarget.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Util/Tuning.h"
#include "Widgets/EdgeTargetWidget.h"
#include "Widgets/ITargetable.h"
#include "Widgets/PointTargetWidget.h"

/// \ingroup Tests
class TargetManagerTest : public SceneTestBase {
  protected:
    /// Derived Widget class that supports receiving target placement.
    class TestTargetableWidget : public Widget, public ITargetable {
      public:
        virtual void PlacePointTarget(const DragInfo &info,
                                      Point3f &position, Vector3f &direction,
                                      Dimensionality &snapped_dims) override {
            position     = info.hit.point;
            direction    = Vector3f(1, 0, 0);
            snapped_dims = Dimensionality("XZ");
        }
        virtual void PlaceEdgeTarget(const DragInfo &info, float current_length,
                                     Point3f &position0,
                                     Point3f &position1) override {
            position0 = info.hit.point;
            position1 = position0 + current_length * Vector3f::AxisY();
        }

      protected:
        TestTargetableWidget() {}

      private:
        friend class Parser::Registry;
    };
    DECL_SHARED_PTR(TestTargetableWidget);

    CommandManagerPtr    cm;
    TargetManager        tm;
    PointTargetWidgetPtr ptw;
    EdgeTargetWidgetPtr  etw;

    /// The constructor sets up the TargetManager with a CommandManager and
    /// installs the target Widgets.
    TargetManagerTest();
};

TargetManagerTest::TargetManagerTest() : cm(new CommandManager), tm(cm) {
    const Str contents = R"(
  children: [<"nodes/Widgets/PointTargetWidget.emd">,
             <"nodes/Widgets/EdgeTargetWidget.emd">])";

    ReadRealScene(contents);

    ptw = SG::FindTypedNodeInScene<PointTargetWidget>(
        *GetScene(), "PointTargetWidget");
    etw = SG::FindTypedNodeInScene<EdgeTargetWidget>(
        *GetScene(), "EdgeTargetWidget");
    tm.InitTargets(ptw, etw);
}

TEST_F(TargetManagerTest, Defaults) {
    auto pt = CreateObject<PointTarget>();
    auto et = CreateObject<EdgeTarget>();

    EXPECT_FALSE(tm.IsPointTargetVisible());
    EXPECT_FALSE(tm.IsEdgeTargetVisible());
    EXPECT_EQ(pt->GetPosition(),  tm.GetPointTarget().GetPosition());
    EXPECT_EQ(pt->GetDirection(), tm.GetPointTarget().GetDirection());
    EXPECT_EQ(pt->GetRadius(),    tm.GetPointTarget().GetRadius());
    EXPECT_EQ(pt->GetArc(),       tm.GetPointTarget().GetArc());
    EXPECT_EQ(et->GetPosition0(), tm.GetEdgeTarget().GetPosition0());
    EXPECT_EQ(et->GetPosition1(), tm.GetEdgeTarget().GetPosition1());
}

TEST_F(TargetManagerTest, Visible) {
    EXPECT_FALSE(tm.IsPointTargetVisible());
    EXPECT_FALSE(tm.IsEdgeTargetVisible());
    tm.SetPointTargetVisible(true);
    EXPECT_TRUE(tm.IsPointTargetVisible());
    EXPECT_FALSE(tm.IsEdgeTargetVisible());
    tm.SetEdgeTargetVisible(true);
    EXPECT_TRUE(tm.IsPointTargetVisible());
    EXPECT_TRUE(tm.IsEdgeTargetVisible());
    tm.SetPointTargetVisible(false);
    EXPECT_FALSE(tm.IsPointTargetVisible());
    EXPECT_TRUE(tm.IsEdgeTargetVisible());
    tm.SetEdgeTargetVisible(false);
    EXPECT_FALSE(tm.IsPointTargetVisible());
    EXPECT_FALSE(tm.IsEdgeTargetVisible());
}

TEST_F(TargetManagerTest, SetTargets) {
    CircleArc arc(Anglef::FromDegrees(20), Anglef::FromDegrees(-100));

    auto pt = CreateObject<PointTarget>();
    pt->SetPosition(Point3f(1, 2, 3));
    pt->SetDirection(Vector3f(0, 0, -4));
    pt->SetRadius(3.5f);
    pt->SetArc(arc);
    tm.SetPointTarget(*pt);
    EXPECT_EQ(Point3f(1, 2, 3),   tm.GetPointTarget().GetPosition());
    EXPECT_EQ(Vector3f(0, 0, -1), tm.GetPointTarget().GetDirection());
    EXPECT_EQ(3.5f,               tm.GetPointTarget().GetRadius());
    EXPECT_EQ(arc,                tm.GetPointTarget().GetArc());

    auto et = CreateObject<EdgeTarget>();
    et->SetPositions(Point3f(1, 2, 3), Point3f(7, 2, 3));
    tm.SetEdgeTarget(*et);
    EXPECT_EQ(Point3f(1, 2, 3),  tm.GetEdgeTarget().GetPosition0());
    EXPECT_EQ(Point3f(7, 2, 3),  tm.GetEdgeTarget().GetPosition1());
}

TEST_F(TargetManagerTest, SnapToPoint) {
    tm.StartSnapping();

    Vector3f motion(1.001f, 0, 0);

    // No snapping while PointTarget is not visible.
    EXPECT_FALSE(tm.SnapToPoint(Point3f(1, 0, 0), motion));
    EXPECT_EQ(Vector3f(1.001f, 0, 0), motion);
    EXPECT_FALSE(tm.SnapToPoint(Point3f(-1, 0, 0), motion));
    EXPECT_EQ(Vector3f(1.001f, 0, 0), motion);

    tm.SetPointTargetVisible(true);
    EXPECT_FALSE(tm.SnapToPoint(Point3f(1, 0, 0), motion));
    EXPECT_EQ(Vector3f(1.001f, 0, 0), motion);
    EXPECT_TRUE(tm.SnapToPoint(Point3f(-1, 0, 0), motion));
    EXPECT_EQ(Vector3f(1, 0, 0), motion);

    tm.EndSnapping();
}

TEST_F(TargetManagerTest, SnapToDirection) {
    tm.StartSnapping();

    const float tol = TK::kSnapDirectionTolerance;
    const auto rot0 = BuildRotation(0, 0, 1, 1.5f * tol);  // Too much.
    const auto rot1 = BuildRotation(0, 0, 1,  .5f * tol);  // Close enough.

    const auto vec0 = rot0 * Vector3f::AxisY();  // Too far.
    const auto vec1 = rot1 * Vector3f::AxisY();  // Close enough.

    Rotationf rot;

    // No snapping while PointTarget is not visible.
    EXPECT_FALSE(tm.SnapToDirection(vec0, rot));
    EXPECT_EQ(Rotationf::Identity(), rot);
    EXPECT_FALSE(tm.SnapToDirection(vec1, rot));
    EXPECT_EQ(Rotationf::Identity(), rot);

    tm.SetPointTargetVisible(true);
    EXPECT_FALSE(tm.SnapToDirection(vec0, rot));
    EXPECT_EQ(Rotationf::Identity(), rot);
    EXPECT_TRUE(tm.SnapToDirection(vec1, rot));
    EXPECT_ROTS_CLOSE(-rot1, rot);

    tm.EndSnapping();
}

TEST_F(TargetManagerTest, SnapToLength) {
    tm.StartSnapping();

    const float tol = TK::kSnapLengthTolerance;
    const auto len0 = 4 + 1.5f * tol;  // Too long.
    const auto len1 = 4 +  .5f * tol;  // Close enough.

    const Dimensionality all_dims("XYZ");

    // Convenience to use the Dimensionality version of SnapToLength() and
    // convert to a string for easy testing.
    auto dim_test = [&](float len){
        return tm.SnapToLength(all_dims,
                               Vector3f(len, len, len)).GetAsString();
    };

    // No snapping while EdgeTarget is not visible.
    EXPECT_FALSE(tm.SnapToLength(len0));
    EXPECT_FALSE(tm.SnapToLength(len1));
    EXPECT_EQ("", dim_test(len0));
    EXPECT_EQ("", dim_test(len1));

    tm.SetEdgeTargetVisible(true);
    EXPECT_FALSE(tm.SnapToLength(len0));
    EXPECT_TRUE(tm.SnapToLength(len1));
    EXPECT_EQ("",    dim_test(len0));
    EXPECT_EQ("XYZ", dim_test(len1));

    tm.EndSnapping();
}

TEST_F(TargetManagerTest, ChangeTargets) {
    // Test that activating and changing the PointTargetWidget or
    // EdgeTargetWidget causes a command to be added to the CommandManager.

    Parser::Registry::AddType<TestTargetableWidget>("TestTargetableWidget");

    // Set up a dummy executor function for the ChangePointTargetCommand and
    // ChangeEdgeTargetCommand. It does not need to do anything.
    auto dummy_func = [](Command &, Command::Op){};
    cm->RegisterFunction("ChangePointTargetCommand", dummy_func);
    cm->RegisterFunction("ChangeEdgeTargetCommand",  dummy_func);

    // TargetManager requires a path to the stage for this to work.
    tm.SetPathToStage(SG::NodePath(GetScene()->GetRootNode()));

    // Detect target activations and deactivations.
    size_t   act_count = 0;
    size_t deact_count = 0;
    auto act_func = [&](bool is_act){
        if (is_act)
            ++act_count;
        else
            ++deact_count;
    };
    tm.GetTargetActivation().AddObserver("key", act_func);

    EXPECT_EQ(0U, act_count);
    EXPECT_EQ(0U, deact_count);
    EXPECT_EQ(0U, cm->GetCommandList()->GetCommandCount());

    // Drag the position of the PointTargetWidget using a TestTargetableWidget.
    // This should create and add a ChangePointTargetCommand.
    auto ttw = CreateObject<TestTargetableWidget>();
    DragTester pdt(ptw, ttw);
    pdt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(2, 0, 0));
    EXPECT_EQ(1U, act_count);
    EXPECT_EQ(1U, deact_count);
    EXPECT_EQ(1U, cm->GetCommandList()->GetCommandCount());
    auto cptc = std::dynamic_pointer_cast<ChangePointTargetCommand>(
        cm->GetCommandList()->GetCommand(0));
    EXPECT_NOT_NULL(cptc);
    EXPECT_EQ(Point3f(2, 0, 0),  cptc->GetNewTarget()->GetPosition());
    EXPECT_EQ(Vector3f(1, 0, 0), cptc->GetNewTarget()->GetDirection());

    cm->ResetCommandList();
    EXPECT_EQ(0U, cm->GetCommandList()->GetCommandCount());

    // Drag the position of the EdgeTargetWidget using a TestTargetableWidget.
    // This should create and add a ChangeEdgeTargetCommand.
    DragTester edt(etw, ttw);
    edt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(2, 0, 0));
    EXPECT_EQ(2U, act_count);
    EXPECT_EQ(2U, deact_count);
    EXPECT_EQ(1U, cm->GetCommandList()->GetCommandCount());
    auto cetc = std::dynamic_pointer_cast<ChangeEdgeTargetCommand>(
        cm->GetCommandList()->GetCommand(0));
    EXPECT_NOT_NULL(cetc);
    EXPECT_EQ(Point3f(2, 0, 0), cetc->GetNewTarget()->GetPosition0());
    EXPECT_EQ(Point3f(2, 4, 0), cetc->GetNewTarget()->GetPosition1());

    cm->ResetCommandList();
    EXPECT_EQ(0U, cm->GetCommandList()->GetCommandCount());

    // Clicking the EdgeTargetWidget should reverse its direction. Note that
    // this does not cause an activation or deactivation.
    etw->Click(ClickInfo());   // Contents of ClickInfo do not matter.
    EXPECT_EQ(2U, act_count);
    EXPECT_EQ(2U, deact_count);
    EXPECT_EQ(1U, cm->GetCommandList()->GetCommandCount());
    cetc = std::dynamic_pointer_cast<ChangeEdgeTargetCommand>(
        cm->GetCommandList()->GetCommand(0));
    EXPECT_NOT_NULL(cetc);
    EXPECT_EQ(Point3f(2, 4, 0), cetc->GetNewTarget()->GetPosition0());
    EXPECT_EQ(Point3f(2, 0, 0), cetc->GetNewTarget()->GetPosition1());
}
