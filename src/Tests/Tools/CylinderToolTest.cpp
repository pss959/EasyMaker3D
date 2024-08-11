//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeCylinderCommand.h"
#include "Models/BoxModel.h"
#include "Models/CylinderModel.h"
#include "Models/RootModel.h"
#include "Place/PrecisionStore.h"
#include "SG/Search.h"
#include "Tools/CylinderTool.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Util/Tuning.h"
#include "Widgets/ScaleWidget.h"
#include "Widgets/Slider1DWidget.h"

DECL_SHARED_PTR(CylinderTool);

/// \ingroup Tests
class CylinderToolTest : public ToolTestBase {
  protected:
    CylinderToolPtr  tool;
    CylinderModelPtr model;       ///< CylinderModel the Tool is attached to.
    ScaleWidgetPtr   top_scaler;  ///< ScaleWidget for the top radius.
    ScaleWidgetPtr   bot_scaler;  ///< ScaleWidget for the bottom radius.

    /// The constructor sets up a CylinderTool attached to a selected
    /// CylinderModel.
    CylinderToolTest();

    /// The destructor detaches the CylinderTool.
    ~CylinderToolTest() { tool->DetachFromSelection(); }
};

CylinderToolTest::CylinderToolTest() {
    // Initialize the CylinderTool.
    tool = InitTool<CylinderTool>("CylinderTool");
    EXPECT_TRUE(tool->IsSpecialized());

    // Access its ScaleWidgets.
    top_scaler = SG::FindTypedNodeUnderNode<ScaleWidget>(
        *tool, "TopRadiusScaler");
    bot_scaler = SG::FindTypedNodeUnderNode<ScaleWidget>(
        *tool, "BottomRadiusScaler");

    // Create and add a CylinderModel.
    model = Model::CreateModel<CylinderModel>("Cylinder");
    context->root_model->AddChildModel(model);

    // Set its radii for consistent testing.
    model->SetTopRadius(2);
    model->SetBottomRadius(3);

    // Attach the CylinderTool to the CylinderModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());

    AddDummyCommandFunction("ChangeCylinderCommand");
}

TEST_F(CylinderToolTest, CanBeUsedFor) {
    // Test that the Tool cannot be attached unless all selected models are
    // CylinderModels.
    tool->DetachFromSelection();

    // Create and add a non-CylinderModel (BoxModel).
    auto box = Model::CreateModel<BoxModel>("Box");
    context->root_model->AddChildModel(box);

    const SelPath box_path(context->root_model, box);
    const SelPath cyl_path(context->root_model, model);

    Selection sel;
    sel.Add(cyl_path);
    sel.Add(box_path);
    EXPECT_FALSE(tool->CanBeUsedFor(sel));

    sel.Clear();
    sel.Add(cyl_path);
    EXPECT_TRUE(tool->CanBeUsedFor(sel));

    // Have to be attached when the destructor is called.
    tool->AttachToSelection(sel, 0);
}

TEST_F(CylinderToolTest, UpdateGripInfo) {
    Grippable::GripInfo info;

    // CylinderTool does not change the color.
    const auto default_color = info.color;

    // This should select the max part of the top scaler.
    info.guide_direction.Set(-1, 0, 0);
    info.event.position3D.Set(0, 1, 0);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kBasic,    info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(2.06f, 1, 0),   info.target_point);
    EXPECT_EQ(top_scaler->GetMaxSlider(),    info.widget);
    EXPECT_EQ(default_color,                 info.color);

    // This should select the min part of the bottom scaler.
    info.guide_direction.Set(1, 1, 0);
    info.event.position3D.Set(0, -.4f, 0);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kBasic,    info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(-3.06f, -1, 0), info.target_point);
    EXPECT_EQ(bot_scaler->GetMinSlider(),    info.widget);
    EXPECT_EQ(default_color,                 info.color);
}

TEST_F(CylinderToolTest, PointerDrag) {
    // Pointer dragging should add a ChangeCylinderCommand.
    CheckNoCommands();

    DragTester dt(top_scaler, StrVec{ "MaxSlider" });
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.5f, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeCylinderCommand>();
    EXPECT_EQ(StrVec{ "Cylinder" }, cmd.GetModelNames());
    EXPECT_TRUE(cmd.IsTopRadius());
    EXPECT_EQ(2.5f, cmd.GetNewRadius());
}

TEST_F(CylinderToolTest, GripDrag) {
    // Grip dragging should add a ChangeCylinderCommand.
    CheckNoCommands();

    DragTester dt(bot_scaler, StrVec{ "MinSlider" });
    dt.ApplyGripDrag(Point3f(0, 0, 0), Point3f(-.1f, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeCylinderCommand>();
    EXPECT_EQ(StrVec{ "Cylinder" }, cmd.GetModelNames());
    EXPECT_FALSE(cmd.IsTopRadius());
    EXPECT_EQ(11, cmd.GetNewRadius());
}

TEST_F(CylinderToolTest, TouchDrag) {
    // Touch dragging should add a ChangeCylinderCommand.
    CheckNoCommands();

    DragTester dt(top_scaler, StrVec{ "MinSlider" });
    dt.ApplyTouchDrag(Point3f(0, 0, 0), Point3f(-1, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeCylinderCommand>();
    EXPECT_EQ(StrVec{ "Cylinder" }, cmd.GetModelNames());
    EXPECT_TRUE(cmd.IsTopRadius());
    EXPECT_EQ(3, cmd.GetNewRadius());
}

TEST_F(CylinderToolTest, SnapRadiusToTarget) {
    CheckNoCommands();

    SetEdgeTargetLength(3);

    DragTester dt(top_scaler, StrVec{ "MaxSlider" });
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.89f, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeCylinderCommand>();
    EXPECT_EQ(StrVec{ "Cylinder" }, cmd.GetModelNames());
    EXPECT_TRUE(cmd.IsTopRadius());
    EXPECT_EQ(3, cmd.GetNewRadius());
}

TEST_F(CylinderToolTest, SnapDiameterToTarget) {
    CheckNoCommands();
    SetEdgeTargetLength(7);

    DragTester dt(bot_scaler, StrVec{ "MaxSlider" });
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.28f, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeCylinderCommand>();
    EXPECT_EQ(StrVec{ "Cylinder" }, cmd.GetModelNames());
    EXPECT_FALSE(cmd.IsTopRadius());
    EXPECT_EQ(3.5f, cmd.GetNewRadius());
}

TEST_F(CylinderToolTest, MinRadius) {
    context->precision_store->Increase();
    const float prec = context->precision_store->GetLinearPrecision();

    // If one radius is 0, the other cannot go down to 0.
    CheckNoCommands();

    model->SetTopRadius(0);
    tool->ReattachToSelection();  // Updates scalers.

    DragTester dt(bot_scaler, StrVec{ "MaxSlider" });
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(-6, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeCylinderCommand>();
    EXPECT_EQ(StrVec{ "Cylinder" }, cmd.GetModelNames());
    EXPECT_FALSE(cmd.IsTopRadius());
    EXPECT_EQ(.5f * prec, cmd.GetNewRadius());
}
