//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include <ion/math/vectorutils.h>

#include "Commands/ChangeMirrorCommand.h"
#include "Models/MirroredModel.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tools/MirrorTool.h"
#include "SG/Search.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/PlaneWidget.h"

DECL_SHARED_PTR(MirrorTool);

/// \ingroup Tests
class MirrorToolTest : public ToolTestBase {
  protected:
    MirrorToolPtr    tool;
    MirroredModelPtr model;   ///< Model the Tool is attached to.
    PlaneWidgetPtr   widget;  ///< PlaneWidget inside the MirrorTool.

    /// The constructor sets up a MirrorTool attached to a selected
    /// MirroredModel.
    MirrorToolTest();

    /// The destructor detaches the MirrorTool.
    ~MirrorToolTest() { tool->DetachFromSelection(); }
};

MirrorToolTest::MirrorToolTest() {
    // Initialize the MirrorTool.
    tool = InitTool<MirrorTool>("MirrorTool");
    EXPECT_TRUE(tool->IsSpecialized());

    // Access its PlaneWidget.
    widget = SG::FindTypedNodeUnderNode<PlaneWidget>(*tool, "PlaneWidget");

    // Create and add a MirroredModel with a BoxModel operand. The BoxModel
    // needs to be big enough for mirror plane movement.
    auto box = Model::CreateModel<BoxModel>("Box");
    box->SetUniformScale(6);
    model = Model::CreateModel<MirroredModel>("Mirrored");
    model->SetOperandModel(box);
    context->root_model->AddChildModel(model);

    // Attach the MirrorTool to the MirroredModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());

    AddDummyCommandFunction("ChangeMirrorCommand");
}

TEST_F(MirrorToolTest, UpdateGripInfo) {
    Grippable::GripInfo info;

    // MirrorTool does not change the color.
    const auto default_color = info.color;

    // Close to plane normal uses the translator.
    info.guide_direction.Set(1, 0, 0);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kBasic,              info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),                 info.target_point);
    EXPECT_EQ(widget->GetSubWidget("PlaneTranslator"), info.widget);
    EXPECT_EQ(default_color,                           info.color);

    // Any other direction uses the plane rotator.
    info.guide_direction.Set(0, 1, 0);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kRotation, info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),       info.target_point);
    EXPECT_EQ("SphereWidget",                info.widget->GetTypeName());
    EXPECT_EQ(default_color,                 info.color);
}

TEST_F(MirrorToolTest, DragPlane) {
    CheckNoCommands();

    DragTester dt(widget, StrVec{ "PlaneTranslator" });
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(0, 2, 0));

    const auto &cmd = CheckOneCommand<ChangeMirrorCommand>();
    EXPECT_EQ(StrVec{ "Mirrored" }, cmd.GetModelNames());
    EXPECT_CLOSE(2,                 cmd.GetPlane().distance);
}

TEST_F(MirrorToolTest, SnapNormalToTarget) {
    CheckNoCommands();

    SetPointTarget(Point3f(0, 0, 0), Vector3f(1, 0, 0));

    // Use grip-dragging here; easier to rotate. Rotate the plane normal around
    // the Z-axis until it is close to the Y-axis.
    DragTester dt(widget, StrVec{ "Axis", "Rotator" });
    dt.ApplyGripRotationDrag(Vector3f(0, -1, 0),
                             BuildRotation(0, 0, 1, 10),
                             BuildRotation(0, 0, 1, 98));

    const auto &cmd = CheckOneCommand<ChangeMirrorCommand>();
    EXPECT_EQ(StrVec{ "Mirrored" }, cmd.GetModelNames());
    EXPECT_EQ(Vector3f(0, 1, 0),    cmd.GetPlane().normal);
}

TEST_F(MirrorToolTest, SnapNormalToPrincipalAxis) {
    CheckNoCommands();

    // Use grip-dragging here; easier to rotate. Rotate the plane normal around
    // the Z-axis until it is close to the -Y-axis.
    DragTester dt(widget, StrVec{ "Axis", "Rotator" });
    dt.ApplyGripRotationDrag(Vector3f(0, -1, 0),
                             BuildRotation(0, 0, 1, -10),
                             BuildRotation(0, 0, 1, -98));

    const auto &cmd = CheckOneCommand<ChangeMirrorCommand>();
    EXPECT_EQ(StrVec{ "Mirrored" }, cmd.GetModelNames());
    EXPECT_EQ(Vector3f(0, -1, 0),   cmd.GetPlane().normal);
}

TEST_F(MirrorToolTest, SnapPlaneToTarget) {
    CheckNoCommands();

    SetPointTarget(Point3f(2, 0, 0), Vector3f(0, 1, 0));

    DragTester dt(widget, StrVec{ "PlaneTranslator" });
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(0, 1.8f, 0));

    const auto &cmd = CheckOneCommand<ChangeMirrorCommand>();
    EXPECT_EQ(StrVec{ "Mirrored" }, cmd.GetModelNames());
    EXPECT_EQ(2,                    cmd.GetPlane().distance);
}
