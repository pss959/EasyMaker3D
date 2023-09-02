#include "Place/DragInfo.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "Tests/TestBaseWithTypes.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class DragInfoTest : public TestBaseWithTypes {};

TEST_F(DragInfoTest, DragInfo) {
    DragInfo info;
    EXPECT_EQ(Trigger::kPointer,      info.trigger);
    EXPECT_FALSE(info.is_modified_mode);
    EXPECT_EQ(0,                      info.linear_precision);
    EXPECT_EQ(0,                      info.angular_precision);
    EXPECT_TRUE(info.path_to_widget.empty());
    EXPECT_TRUE(info.path_to_stage.empty());
    EXPECT_FALSE(info.hit.IsValid());
    EXPECT_EQ(Vector3f::Zero(),       info.grip_guide_direction);
    EXPECT_EQ(Point3f::Zero(),        info.grip_position);
    EXPECT_EQ(Rotationf::Identity(),  info.grip_orientation);
    EXPECT_EQ(Point3f::Zero(),        info.touch_position);

    // Set up paths for testing functions.
    auto root   = CreateObject<SG::Node>("Root");
    auto stage  = CreateObject<SG::Node>("Stage");
    auto widget = CreateObject<SG::Node>("Widget");
    auto part   = CreateObject<SG::Node>("Part");
    root->AddChild(stage);
    stage->AddChild(widget);
    widget->AddChild(part);
    info.path_to_stage.push_back(root);
    info.path_to_stage.push_back(stage);
    info.path_to_widget = info.path_to_stage;
    info.path_to_widget.push_back(widget);
    info.hit.path = info.path_to_widget;
    info.hit.path.push_back(part);

    EXPECT_EQ(Matrix4f::Identity(),   info.GetWorldToStageMatrix());
    EXPECT_EQ(Matrix4f::Identity(),   info.GetObjectToStageMatrix());
    EXPECT_EQ(Point3f::Zero(),        info.GetLocalGripPosition());

    // Set some transforms and try again.
    stage->SetTranslation(Vector3f(1, 2, 3));
    widget->SetTranslation(Vector3f(10, 20, 30));
    part->SetTranslation(Vector3f(100, 200, 300));
    EXPECT_EQ(Matrix4f(1, 0, 0, -1, 0, 1, 0, -2, 0, 0, 1, -3, 0, 0, 0, 1),
              info.GetWorldToStageMatrix());
    EXPECT_EQ(Matrix4f(1, 0, 0, 110, 0, 1, 0, 220, 0, 0, 1, 330, 0, 0, 0, 1),
              info.GetObjectToStageMatrix());
    EXPECT_EQ(Point3f(-1, -2, -3), info.GetLocalGripPosition());
}

TEST_F(DragInfoTest, Equality) {
    const DragInfo info0;
    const DragInfo info1;
    DragInfo       info2;

    info2.is_modified_mode = true;
    EXPECT_EQ(info0, info0);
    EXPECT_EQ(info0, info1);
    EXPECT_NE(info0, info2);
}
