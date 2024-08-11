//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ScaleCommand.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "SG/Search.h"
#include "Tools/ScaleTool.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/ScaleWidget.h"
#include "Widgets/Slider1DWidget.h"

DECL_SHARED_PTR(ScaleTool);

/// \ingroup Tests
class ScaleToolTest : public ToolTestBase {
  protected:
    ScaleToolPtr tool;
    ModelPtr        model;   ///< Model the Tool is attached to.

    /// The constructor sets up a ScaleTool attached to a selected
    /// BoxModel.
    ScaleToolTest();

    /// The destructor detaches the ScaleTool.
    ~ScaleToolTest() { tool->DetachFromSelection(); }

    /// Returns the min or max DraggableWidget inside the named ScaleWidget.
    DraggableWidgetPtr FindWidget(const Str &name, bool is_min) {
        auto sw = SG::FindTypedNodeUnderNode<ScaleWidget>(*tool, name);
        return is_min ? sw->GetMinSlider() : sw->GetMaxSlider();
    }
};

ScaleToolTest::ScaleToolTest() {
    // Initialize the ScaleTool.
    tool = InitTool<ScaleTool>("ScaleTool");
    EXPECT_FALSE(tool->IsSpecialized());

    // Create and add a BoxModel.
    model = Model::CreateModel<BoxModel>("Box");
    context->root_model->AddChildModel(model);

    // Attach the ScaleTool to the BoxModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());

    AddDummyCommandFunction("ScaleCommand");
}

TEST_F(ScaleToolTest, UpdateGripInfo) {
    Grippable::GripInfo info;

    const auto default_color = info.color;

    // Not close enough to anything.
    info.guide_direction = Normalized(1, 2, 4);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kBasic,          info.guide_type);
    EXPECT_NULL(info.widget);
    EXPECT_EQ(default_color,                       info.color);

    // 1D scaler.
    info.guide_direction.Set(1, 0, 0);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kBasic,          info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(-2.1f, 0, 0),         info.target_point);
    EXPECT_EQ(FindWidget("XScaler", true),         info.widget);
    EXPECT_EQ("#dd595bff",                         info.color.ToHexString());

    // 2D scaler.
    info.guide_direction = Normalized(1, -1, 0);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kBasic,          info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(-1.1f, 1.1f, 0),      info.target_point);
    EXPECT_EQ(FindWidget("XY2Scaler", false),      info.widget);
    EXPECT_EQ(default_color,                       info.color);

    // 3D scaler.
    info.guide_direction = Normalized(1, 1, 1);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kBasic,          info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(-1.1f, -1.1f, -1.1f), info.target_point);
    EXPECT_EQ(FindWidget("XYZ4Scaler", true),      info.widget);
    EXPECT_EQ(default_color,                       info.color);
}

TEST_F(ScaleToolTest, ScaleAsymmetric) {
    CheckNoCommands();

    DragTester dt(FindWidget("XScaler", false));
    dt.SetRayDirection(Vector3f(0, -1, 0));
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(4, 0, 0));

    const auto &cmd = CheckOneCommand<ScaleCommand>();
    EXPECT_EQ(StrVec{ "Box" },                      cmd.GetModelNames());
    EXPECT_ENUM_EQ(ScaleCommand::Mode::kAsymmetric, cmd.GetMode());
    EXPECT_EQ(Vector3f(2, 1, 1),                    cmd.GetRatios());
}

TEST_F(ScaleToolTest, ScaleBaseSymmetric) {
    CheckNoCommands();

    DragTester dt(FindWidget("XScaler", false));
    context->is_modified_mode = true;
    dt.SetIsModifiedMode(true);  // Turns on symmetric flag.
    dt.SetRayDirection(Vector3f(0, -1, 0));
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(4, 0, 0));

    const auto &cmd = CheckOneCommand<ScaleCommand>();
    EXPECT_EQ(StrVec{ "Box" },                         cmd.GetModelNames());
    EXPECT_ENUM_EQ(ScaleCommand::Mode::kBaseSymmetric, cmd.GetMode());
    EXPECT_EQ(Vector3f(3, 1, 1),                       cmd.GetRatios());
}

TEST_F(ScaleToolTest, ScaleCenterSymmetric) {
    CheckNoCommands();

    // Move the BoxModel up so it is not on the Stage.
    model->SetTranslation(model->GetTranslation() + Vector3f(0, 2, 0));

    DragTester dt(FindWidget("XScaler", true));
    context->is_modified_mode = true;
    dt.SetIsModifiedMode(true);  // Turns on symmetric flag.
    dt.SetRayDirection(Vector3f(0, -1, 0));
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(-4, 0, 0));

    const auto &cmd = CheckOneCommand<ScaleCommand>();
    EXPECT_EQ(StrVec{ "Box" },                           cmd.GetModelNames());
    EXPECT_ENUM_EQ(ScaleCommand::Mode::kCenterSymmetric, cmd.GetMode());
    EXPECT_EQ(Vector3f(3, 1, 1),                         cmd.GetRatios());
}

TEST_F(ScaleToolTest, SnapToTarget1D) {
    CheckNoCommands();

    SetEdgeTargetLength(4);

    DragTester dt(FindWidget("YScaler", false));
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(4.05f, 0, 0));

    const auto &cmd = CheckOneCommand<ScaleCommand>();
    EXPECT_EQ(StrVec{ "Box" },                      cmd.GetModelNames());
    EXPECT_ENUM_EQ(ScaleCommand::Mode::kAsymmetric, cmd.GetMode());
    EXPECT_EQ(Vector3f(1, 2, 1),                    cmd.GetRatios());
}

TEST_F(ScaleToolTest, SnapToTarget3D) {
    CheckNoCommands();

    SetEdgeTargetLength(4);

    DragTester dt(FindWidget("XYZ3Scaler", false));
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(4.05f, 4.05f, 4.05f));

    const auto &cmd = CheckOneCommand<ScaleCommand>();
    EXPECT_EQ(StrVec{ "Box" },                      cmd.GetModelNames());
    EXPECT_ENUM_EQ(ScaleCommand::Mode::kAsymmetric, cmd.GetMode());
    EXPECT_EQ(Vector3f(2, 2, -2),                   cmd.GetRatios());
}
