//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeBendCommand.h"
#include "Models/BentModel.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tools/BendTool.h"
#include "SG/Search.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/SpinWidget.h"

DECL_SHARED_PTR(BendTool);

/// \ingroup Tests
class BendToolTest : public ToolTestBase {
  protected:
    BendToolPtr   tool;
    BentModelPtr  model;   ///< Model the Tool is attached to.
    SpinWidgetPtr widget;  ///< SpinWidget inside the BendTool.

    /// The constructor sets up a BendTool attached to a selected BentModel.
    BendToolTest();

    /// The destructor detaches the BendTool.
    ~BendToolTest() { tool->DetachFromSelection(); }
};

BendToolTest::BendToolTest() {
    // Initialize the BendTool.
    tool = InitTool<BendTool>("BendTool");
    EXPECT_TRUE(tool->IsSpecialized());

    // Access its SpinWidget.
    widget = SG::FindTypedNodeUnderNode<SpinWidget>(*tool, "SpinWidget");

    // Create and add a BentModel with a BoxModel operand.
    model = Model::CreateModel<BentModel>("Bent");
    model->SetOperandModel(Model::CreateModel<BoxModel>("Box"));
    context->root_model->AddChildModel(model);

    // Attach the BendTool to the BentModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());

    AddDummyCommandFunction("ChangeBendCommand");
}

TEST_F(BendToolTest, UpdateGripInfo) {
    Grippable::GripInfo info;

    // BendTool does not change the color.
    const auto default_color = info.color;

    // Close to axis direction uses the axis rotator.
    info.guide_direction.Set(0, 1, 0);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kRotation,   info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),         info.target_point);
    EXPECT_EQ(widget->GetSubWidget("Rotator"), info.widget);
    EXPECT_EQ(default_color,                   info.color);

    // Close to perpendicular uses the axis translator.
    info.guide_direction.Set(1, 0, 0);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kBasic,         info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),            info.target_point);
    EXPECT_EQ(widget->GetSubWidget("Translator"), info.widget);
    EXPECT_EQ(default_color,                      info.color);

    // Any other direction uses the spin ring.
    info.guide_direction = Normalized(1, 1, 1);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kRotation, info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),       info.target_point);
    EXPECT_EQ(widget->GetSubWidget("Ring"),  info.widget);
    EXPECT_EQ(default_color,                 info.color);
}

TEST_F(BendToolTest, DragCenter) {
    CheckNoCommands();

    DragTester dt(widget, StrVec{ "Translator" });
    dt.SetRayDirection(-Vector3f::AxisY());
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(1, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeBendCommand>();
    EXPECT_EQ(StrVec{ "Bent" }, cmd.GetModelNames());
    EXPECT_EQ(Point3f(1, 0, 0), cmd.GetSpin().center);
}

TEST_F(BendToolTest, DragAxis) {
    CheckNoCommands();

    // Use grip-dragging here; easier to rotate.
    DragTester dt(widget, StrVec{ "Axis", "Rotator" });
    dt.ApplyGripRotationDrag(Vector3f(0, -1, 0),
                             BuildRotation(0, 0, 1,  10),
                             BuildRotation(0, 0, 1, 100));

    const auto &cmd = CheckOneCommand<ChangeBendCommand>();
    EXPECT_EQ(StrVec{ "Bent" },   cmd.GetModelNames());
    EXPECT_EQ(Vector3f(-1, 0, 0), cmd.GetSpin().axis);
}

TEST_F(BendToolTest, DragAngle) {
    CheckNoCommands();

    // Use grip-dragging here; easier to rotate.
    DragTester dt(widget, StrVec{ "Ring" });
    dt.ApplyGripRotationDrag(Normalized(1, 1, 1),
                             BuildRotation(1, 0, 0, 0),
                             BuildRotation(1, 0, 0, 45));

    const auto &cmd = CheckOneCommand<ChangeBendCommand>();
    EXPECT_EQ(StrVec{ "Bent" }, cmd.GetModelNames());
    EXPECT_CLOSE(45,            cmd.GetSpin().angle.Degrees());
}

TEST_F(BendToolTest, DragOffset) {
    CheckNoCommands();

    DragTester dt(widget, StrVec{ "Offset" });
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(0, 1, 0));

    const auto &cmd = CheckOneCommand<ChangeBendCommand>();
    EXPECT_EQ(StrVec{ "Bent" }, cmd.GetModelNames());
    EXPECT_CLOSE(1,             cmd.GetSpin().offset);
}

TEST_F(BendToolTest, SnapAxisToTarget) {
    CheckNoCommands();

    SetPointTarget(Point3f(0, 0, 0), Vector3f(1, 0, 0));

    // Use grip-dragging here; easier to rotate. Rotate the bend axis around
    // the Z-axis until it is close to the X-axis.
    DragTester dt(widget, StrVec{ "Axis", "Rotator" });
    dt.ApplyGripRotationDrag(Vector3f(0, -1, 0),
                             BuildRotation(0, 0, 1, 10),
                             BuildRotation(0, 0, 1, 98));

    const auto &cmd = CheckOneCommand<ChangeBendCommand>();
    EXPECT_EQ(StrVec{ "Bent" },  cmd.GetModelNames());
    EXPECT_EQ(Vector3f(1, 0, 0), cmd.GetSpin().axis);
}


TEST_F(BendToolTest, SnapCenterToTarget) {
    CheckNoCommands();

    SetPointTarget(Point3f(2, 0, 0), Vector3f(0, 1, 0));

    DragTester dt(widget, StrVec{ "Axis", "Translator" });
    dt.SetRayDirection(-Vector3f::AxisY());
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(1.9f, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeBendCommand>();
    EXPECT_EQ(StrVec{ "Bent" }, cmd.GetModelNames());
    EXPECT_EQ(Point3f(2, 0, 0), cmd.GetSpin().center);
}
