#include <ion/math/vectorutils.h>

#include "Commands/ChangeClipCommand.h"
#include "Models/ClippedModel.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tools/ClipTool.h"
#include "SG/Search.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/PlaneWidget.h"

DECL_SHARED_PTR(ClipTool);

/// \ingroup Tests
class ClipToolTest : public ToolTestBase {
  protected:
    ClipToolPtr      tool;
    ClippedModelPtr  model;   ///< Model the Tool is attached to.
    PlaneWidgetPtr   widget;  ///< PlaneWidget inside the ClipTool.

    /// The constructor sets up a ClipTool attached to a selected ClippedModel.
    ClipToolTest();

    /// The destructor detaches the ClipTool.
    ~ClipToolTest() { tool->DetachFromSelection(); }
};

ClipToolTest::ClipToolTest() {
    // Initialize the ClipTool.
    tool = InitTool<ClipTool>("ClipTool");
    EXPECT_TRUE(tool->IsSpecialized());

    // Access its PlaneWidget.
    widget = SG::FindTypedNodeUnderNode<PlaneWidget>(*tool, "PlaneWidget");

    // Create and add a ClippedModel with a BoxModel operand. The BoxModel
    // needs to be big enough for clip plane movement.
    auto box = Model::CreateModel<BoxModel>("Box");
    box->SetUniformScale(6);
    model = Model::CreateModel<ClippedModel>("Clipped");
    model->SetOperandModel(box);
    context->root_model->AddChildModel(model);

    // Attach the ClipTool to the ClippedModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());

    AddDummyCommandFunction("ChangeClipCommand");
}

TEST_F(ClipToolTest, UpdateGripInfo) {
    Grippable::GripInfo info;

    // ClipTool does not change the color.
    const auto default_color = info.color;

    // Close to plane normal uses the translator.
    info.guide_direction.Set(0, 1, 0);
    tool->UpdateGripInfo(info);
    EXPECT_EQ(GripGuideType::kBasic,                   info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),                 info.target_point);
    EXPECT_EQ(widget->GetSubWidget("PlaneTranslator"), info.widget);
    EXPECT_EQ(default_color,                           info.color);

    // Any other direction uses the plane rotator.
    info.guide_direction.Set(1, 0, 0);
    tool->UpdateGripInfo(info);
    EXPECT_EQ(GripGuideType::kRotation, info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),  info.target_point);
    EXPECT_EQ("SphereWidget",           info.widget->GetTypeName());
    EXPECT_EQ(default_color,            info.color);
}

TEST_F(ClipToolTest, DragPlane) {
    CheckNoCommands();

    DragTester dt(widget, StrVec{ "PlaneTranslator" });
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(0, 2, 0));

    const auto &cmd = CheckOneCommand<ChangeClipCommand>();
    EXPECT_EQ(2, cmd.GetPlane().distance);
}

TEST_F(ClipToolTest, SnapNormalToTarget) {
    CheckNoCommands();

    SetPointTarget(Point3f(0, 0, 0), Vector3f(1, 0, 0));

    // Use grip-dragging here; easier to rotate. Rotate the plane normal around
    // the Z-axis until it is close to the X-axis.
    DragTester dt(widget, StrVec{ "Axis", "Rotator" });
    dt.ApplyGripRotationDrag(Vector3f(0, -1, 0),
                             BuildRotation(0, 0, 1, 10),
                             BuildRotation(0, 0, 1, 98));

    const auto &cmd = CheckOneCommand<ChangeClipCommand>();
    EXPECT_EQ(Vector3f(1, 0, 0), cmd.GetPlane().normal);
}

TEST_F(ClipToolTest, SnapNormalToPrincipalAxis) {
    CheckNoCommands();

    // Use grip-dragging here; easier to rotate. Rotate the plane normal around
    // the Z-axis until it is close to the -X-axis.
    DragTester dt(widget, StrVec{ "Axis", "Rotator" });
    dt.ApplyGripRotationDrag(Vector3f(0, -1, 0),
                             BuildRotation(0, 0, 1, 10),
                             BuildRotation(0, 0, 1, 98));

    const auto &cmd = CheckOneCommand<ChangeClipCommand>();
    EXPECT_EQ(Vector3f(-1, 0, 0), cmd.GetPlane().normal);
}

TEST_F(ClipToolTest, SnapPlaneToTarget) {
    CheckNoCommands();

    SetPointTarget(Point3f(0, 2, 0), Vector3f(0, 1, 0));

    DragTester dt(widget, StrVec{ "PlaneTranslator" });
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(0, 1.8f, 0));

    const auto &cmd = CheckOneCommand<ChangeClipCommand>();
    EXPECT_EQ(2, cmd.GetPlane().distance);
}
