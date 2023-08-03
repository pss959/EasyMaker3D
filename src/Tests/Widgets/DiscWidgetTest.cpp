#include "Math/Linear.h"
#include "Parser/Exception.h"
#include "Place/DragInfo.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Widgets/DiscWidget.h"

// XXXX Class used to set up DragInfo instances for testing Widgets.
class DragTester {
  public:
    DragTester() {
        base_info_.ray.direction        = -Vector3f::AxisZ();
        base_info_.grip_guide_direction = -Vector3f::AxisX();
        base_info_.grip_position        =  Point3f::Zero();
    }
    void SetLinearPrecision(float p)  { base_info_.linear_precision  = p; }
    void SetAngularPrecision(float p) { base_info_.angular_precision = p; }
    void SetPathToWidget(const SG::NodePath &path) {
        base_info_.path_to_widget = path;
        base_info_.hit.path       = path;
    }
    void SetRayDirection(const Vector3f &dir) {
        base_info_.ray.direction = dir;
    }

    // Adds DragInfo instances for a mouse drag between two points in local
    // coordinates of the Widget.
    void AddMouseDrag(const Point3f &p0, const Point3f &p1,
                      size_t count_between = 0);

    // Adds DragInfo instances for a grip rotation drag.
    void AddGripRotationDrag(const Vector3f &guide_dir,
                             const Rotationf &r0, const Rotationf &r1,
                             size_t count_between = 0);

    // Returns all DragInfo instances.
    const std::vector<DragInfo> & GetDragInfos() const { return infos_; }

    // Clears infos.
    void Clear() { infos_.clear(); }

  private:
    DragInfo              base_info_;
    std::vector<DragInfo> infos_;
};

void DragTester::AddMouseDrag(const Point3f &p0, const Point3f &p1,
                              size_t count_between) {
    DragInfo info = base_info_;
    info.trigger  = Trigger::kPointer;

    const SG::CoordConv cc(info.path_to_widget);

    // Sets the given point to store in the SG::Hit and also computes the origin
    // point for the Ray based on the Ray direction. Adds the resulting
    // DragInfo to the infos_ vector.
    const auto add_pt = [&](const Point3f &p){
        info.hit.point = p;
        info.ray.origin = cc.LocalToRoot(p) - 10 * info.ray.direction;
        infos_.push_back(info);
    };

    // Drag start.
    add_pt(p0);

    // Intermediate points.
    const float delta = 1.f / (count_between + 1);
    for (size_t i = 0; i < count_between; ++i)
        add_pt(Lerp((i + 1) * delta, p0, p1));

    // End of drag.
    add_pt(p1);
}

void DragTester::AddGripRotationDrag(const Vector3f &guide_dir,
                                     const Rotationf &r0, const Rotationf &r1,
                                     size_t count_between) {
    DragInfo info = base_info_;
    info.trigger  = Trigger::kGrip;
    info.grip_guide_direction = guide_dir;

    const SG::CoordConv cc(info.path_to_widget);

    // Sets the given point to store in the SG::Hit and also computes the origin
    // point for the Ray based on the Ray direction. Adds the resulting
    // DragInfo to the infos_ vector.
    const auto add_rot = [&](const Rotationf &r){
        info.grip_orientation = r;
        infos_.push_back(info);
    };

    // Drag start.
    add_rot(r0);

    // Intermediate points.
    const float delta = 1.f / (count_between + 1);
    for (size_t i = 0; i < count_between; ++i)
        add_rot(Rotationf::Slerp(r0, r1, (i + 1) * delta));

    // End of drag.
    add_rot(r1);
}

// ----------------------------------------------------------------------------

class DiscWidgetTest : public SceneTestBase {};

TEST_F(DiscWidgetTest, Defaults) {
    auto dw = CreateObject<DiscWidget>();
    EXPECT_EQ(DiscWidget::Mode::kRotateAndScale, dw->GetMode());
    EXPECT_EQ(DiscWidget::AngleMode::kClamp,     dw->GetAngleMode());
    EXPECT_EQ(0,                           dw->GetRotationAngle().Degrees());
    EXPECT_EQ(1,                           dw->GetScaleFactor());
}

TEST_F(DiscWidgetTest, RotationInField) {
    TEST_THROW(ReadTypedItem<DiscWidget>("DiscWidget { rotation: 0 1 0 10 }"),
               Parser::Exception, "Rotation will be ignored");
}


TEST_F(DiscWidgetTest, NoMotion) {
    auto dw = CreateObject<DiscWidget>();

    DragTester dt;
    dt.SetPathToWidget(SG::NodePath(dw));
    dt.SetRayDirection(-Vector3f::AxisY());
    dt.AddMouseDrag(Point3f(0, 0, 2), Point3f(0.0001f, 0, 2));
    {
        const auto &infos = dt.GetDragInfos();
        EXPECT_EQ(2U, infos.size());
        EXPECT_EQ(1,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
        EXPECT_EQ(0,                 dw->GetRotationAngle().Degrees());
        dw->StartDrag(infos[0]);
        dw->ContinueDrag(infos[1]);
        dw->EndDrag();
        EXPECT_EQ(1,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
        EXPECT_EQ(0,                 dw->GetRotationAngle().Degrees());
    }
}

TEST_F(DiscWidgetTest, DragScale) {
    auto dw = CreateObject<DiscWidget>();

    // Dragging from Z=2 to Z=4 should scale by 2.
    DragTester dt;
    dt.SetPathToWidget(SG::NodePath(dw));
    dt.SetRayDirection(-Vector3f::AxisY());
    dt.AddMouseDrag(Point3f(0, 0, 2), Point3f(0, 0, 4));
    {
        const auto &infos = dt.GetDragInfos();
        EXPECT_EQ(2U, infos.size());
        EXPECT_EQ(1,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
        EXPECT_EQ(0,                 dw->GetRotationAngle().Degrees());
        dw->StartDrag(infos[0]);
        dw->ContinueDrag(infos[1]);
        dw->EndDrag();
        EXPECT_EQ(2,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(2, 2, 2), dw->GetScale());
        EXPECT_EQ(0,                 dw->GetRotationAngle().Degrees());
    }

    // Dragging from X=3 to X=9 should scale by 3. Add some intermediate points
    // to make sure it works.
    dt.Clear();
    dt.AddMouseDrag(Point3f(3, 0, 0), Point3f(9, 0, 0), 4);
    {
        const auto &infos = dt.GetDragInfos();
        EXPECT_EQ(6U, infos.size());
        EXPECT_EQ(2,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(2, 2, 2), dw->GetScale());
        EXPECT_EQ(0,                 dw->GetRotationAngle().Degrees());
        dw->StartDrag(infos[0]);
        for (int i = 1; i < 6; ++i)
            dw->ContinueDrag(infos[i]);
        dw->EndDrag();
        EXPECT_EQ(3,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(6, 6, 6), dw->GetScale());
        EXPECT_EQ(0,                 dw->GetRotationAngle().Degrees());
    }
}

TEST_F(DiscWidgetTest, DragRotate) {
    auto dw = CreateObject<DiscWidget>();

    // Dragging along +X offset from center should rotate counterclockwise.
    DragTester dt;
    dt.SetPathToWidget(SG::NodePath(dw));
    dt.SetRayDirection(-Vector3f::AxisY());
    dt.AddMouseDrag(Point3f(0, 0, 2), Point3f(2, 0, 2));
    {
        const auto &infos = dt.GetDragInfos();
        EXPECT_EQ(2U, infos.size());
        EXPECT_EQ(1,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
        EXPECT_EQ(0,                 dw->GetRotationAngle().Degrees());
        dw->StartDrag(infos[0]);
        dw->ContinueDrag(infos[1]);
        dw->EndDrag();
        EXPECT_EQ(1,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
        EXPECT_EQ(45,                dw->GetRotationAngle().Degrees());
    }
}

TEST_F(DiscWidgetTest, DragEdgeOnRotate) {
    auto dw = CreateObject<DiscWidget>();

    // Dragging along +X with the ray along -Z should do edge-on rotation.
    DragTester dt;
    dt.SetPathToWidget(SG::NodePath(dw));
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.AddMouseDrag(Point3f(-4, 0, 4), Point3f(4, 0, 4));
    {
        const auto &infos = dt.GetDragInfos();
        EXPECT_EQ(2U, infos.size());
        EXPECT_EQ(1,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
        EXPECT_EQ(0,                 dw->GetRotationAngle().Degrees());
        dw->StartDrag(infos[0]);
        dw->ContinueDrag(infos[1]);
        dw->EndDrag();
        EXPECT_EQ(1,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
        EXPECT_EQ(90,                dw->GetRotationAngle().Degrees());
    }

    // Same test, but drag off the cylinder.
    dt.Clear();
    dt.AddMouseDrag(Point3f(-4, 0, 4), Point3f(10, 0, 4), 4);
    {
        const auto &infos = dt.GetDragInfos();
        EXPECT_EQ(6U, infos.size());
        EXPECT_EQ(1,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
        EXPECT_EQ(90,                 dw->GetRotationAngle().Degrees());
        dw->StartDrag(infos[0]);
        for (int i = 1; i < 6; ++i)
            dw->ContinueDrag(infos[i]);
        dw->EndDrag();
        EXPECT_EQ(1,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
        EXPECT_CLOSE(186.0612f,      dw->GetRotationAngle().Degrees());
    }
}

TEST_F(DiscWidgetTest, DragScaleOnly) {
    auto dw = ReadTypedItem<DiscWidget>("DiscWidget { mode: \"kScaleOnly\" }");
    EXPECT_EQ(DiscWidget::Mode::kScaleOnly, dw->GetMode());

    // Dragging from Z=2 to Z=4 should scale by 2.
    DragTester dt;
    dt.SetPathToWidget(SG::NodePath(dw));
    dt.SetRayDirection(-Vector3f::AxisY());
    dt.AddMouseDrag(Point3f(0, 0, 2), Point3f(0, 0, 4));
    {
        const auto &infos = dt.GetDragInfos();
        EXPECT_EQ(2U, infos.size());
        EXPECT_EQ(1,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
        EXPECT_EQ(0,                 dw->GetRotationAngle().Degrees());
        dw->StartDrag(infos[0]);
        dw->ContinueDrag(infos[1]);
        dw->EndDrag();
        EXPECT_EQ(2,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(2, 2, 2), dw->GetScale());
    }
}

TEST_F(DiscWidgetTest, DragRotateOnly) {
    auto dw =
        ReadTypedItem<DiscWidget>("DiscWidget { mode: \"kRotationOnly\" }");

    // Dragging along +X offset from center should rotate counterclockwise.
    DragTester dt;
    dt.SetPathToWidget(SG::NodePath(dw));
    dt.SetRayDirection(-Vector3f::AxisY());
    dt.AddMouseDrag(Point3f(0, 0, 2), Point3f(2, 0, 2));
    {
        const auto &infos = dt.GetDragInfos();
        EXPECT_EQ(2U, infos.size());
        EXPECT_EQ(1,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
        EXPECT_EQ(0,                 dw->GetRotationAngle().Degrees());
        dw->StartDrag(infos[0]);
        dw->ContinueDrag(infos[1]);
        dw->EndDrag();
        EXPECT_EQ(1,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
        EXPECT_EQ(45,                dw->GetRotationAngle().Degrees());
    }
}

TEST_F(DiscWidgetTest, GripRotate) {
    auto dw = CreateObject<DiscWidget>();

    DragTester dt;
    dt.SetPathToWidget(SG::NodePath(dw));
    dt.AddGripRotationDrag(Vector3f::AxisX(),
                           BuildRotation(1, 0, 0, 20),
                           BuildRotation(1, 0, 0, 100));
    {
        const auto &infos = dt.GetDragInfos();
        EXPECT_EQ(2U, infos.size());
        EXPECT_EQ(1,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
        EXPECT_EQ(0,                 dw->GetRotationAngle().Degrees());
        dw->StartDrag(infos[0]);
        dw->ContinueDrag(infos[1]);
        dw->EndDrag();
        EXPECT_EQ(1,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
        EXPECT_CLOSE(80,             dw->GetRotationAngle().Degrees());
    }

    // Repeat with opposite guide direction.
    dt.Clear();
    dt.AddGripRotationDrag(-Vector3f::AxisX(),
                           BuildRotation(1, 0, 0, 20),
                           BuildRotation(1, 0, 0, 100));
    dw->SetRotationAngle(Anglef::FromDegrees(0));
    {
        const auto &infos = dt.GetDragInfos();
        EXPECT_EQ(2U, infos.size());
        EXPECT_EQ(1,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
        EXPECT_EQ(0,                 dw->GetRotationAngle().Degrees());
        dw->StartDrag(infos[0]);
        dw->ContinueDrag(infos[1]);
        dw->EndDrag();
        EXPECT_EQ(1,                 dw->GetScaleFactor());
        EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
        EXPECT_CLOSE(-80,            dw->GetRotationAngle().Degrees());
    }
}

TEST_F(DiscWidgetTest, ClampAngle) {
    auto dw = CreateObject<DiscWidget>();

    // There is no way to create a large enough angle with a mouse drag, so use
    // a wrist-breaking grip rotation.
    DragTester dt;
    dt.SetPathToWidget(SG::NodePath(dw));

    auto test_rot = [&](float expected_deg, float rot_deg0, float rot_deg1){
        dt.Clear();
        dt.AddGripRotationDrag(Vector3f::AxisX(),
                               BuildRotation(1, 0, 0, 0),
                               BuildRotation(1, 0, 0, rot_deg0));
        dt.AddGripRotationDrag(Vector3f::AxisX(),
                               BuildRotation(1, 0, 0, rot_deg0),
                               BuildRotation(1, 0, 0, rot_deg1));
        {
            const auto &infos = dt.GetDragInfos();
            EXPECT_EQ(4U, infos.size());
            dw->StartDrag(infos[0]);
            for (int i = 1; i < 4; ++i)
                dw->ContinueDrag(infos[i]);
            dw->EndDrag();
            EXPECT_CLOSE(expected_deg, dw->GetRotationAngle().Degrees());
        }
    };

    // Start in the positive direction.
    test_rot(240, 120, -120);   // > 180 difference.

    // Start in the negative direction.
    dw->SetRotationAngle(Anglef::FromDegrees(0));
    test_rot(-220, -130, 140);  // < -180 difference.
}

TEST_F(DiscWidgetTest, AccumulateAngle) {
    auto dw =
        ReadTypedItem<DiscWidget>("DiscWidget { angle_mode: \"kAccumulate\" }");

    // Similar to the above test, but the angle should accumulate.
    // Positive direction.
    DragTester dt;
    dt.SetPathToWidget(SG::NodePath(dw));

    auto test_rot = [&](float expected_deg, float rot_deg){
        dt.Clear();
        dt.AddGripRotationDrag(Vector3f::AxisX(),
                               BuildRotation(1, 0, 0, 0),
                               BuildRotation(1, 0, 0, rot_deg));
        {
            const auto &infos = dt.GetDragInfos();
            EXPECT_EQ(2U, infos.size());
            dw->StartDrag(infos[0]);
            dw->ContinueDrag(infos[1]);
            dw->EndDrag();
            EXPECT_CLOSE(expected_deg, dw->GetRotationAngle().Degrees());
        }
    };

    // Positive direction.
    test_rot(120, 120);
    test_rot(280, 160);
    test_rot(400, 120);
    test_rot(380, -20);
    test_rot(370, 350);

    // Negative direction.
    dw->SetRotationAngle(Anglef::FromDegrees(0));
    test_rot(-110, -110);
    test_rot(-270, -160);
    test_rot(-400, -130);
    test_rot(-380,   20);
    test_rot(-370, -350);
}
