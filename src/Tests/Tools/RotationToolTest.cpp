#include "Commands/RotateCommand.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "SG/Search.h"
#include "Tools/RotationTool.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/DraggableWidget.h"

DECL_SHARED_PTR(RotationTool);

/// \ingroup Tests
class RotationToolTest : public ToolTestBase {
  protected:
    RotationToolPtr tool;
    ModelPtr        model;   ///< Model the Tool is attached to.

    /// The constructor sets up a RotationTool attached to a selected
    /// BoxModel.
    RotationToolTest();

    /// The destructor detaches the RotationTool.
    ~RotationToolTest() { tool->DetachFromSelection(); }

    /// Returns the named rotation DraggableWidget.
    DraggableWidgetPtr FindRotator(const Str &name) {
        return SG::FindTypedNodeUnderNode<DraggableWidget>(*tool, name);
    }
};

RotationToolTest::RotationToolTest() {
    // Initialize the RotationTool.
    tool = InitTool<RotationTool>("RotationTool");
    EXPECT_FALSE(tool->IsSpecialized());

    // Create and add a BoxModel.
    model = Model::CreateModel<BoxModel>("Box");
    context->root_model->AddChildModel(model);

    // Attach the RotationTool to the BoxModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());

    AddDummyCommandFunction("RotateCommand");
}

TEST_F(RotationToolTest, UpdateGripInfo) {
    Grippable::GripInfo info;

    // RotationTool does not change the color.
    const auto default_color = info.color;

    // Close to any principal axis chooses the axis rotator.
    info.guide_direction.Set(1, 0, 0);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kRotation, info.guide_type);
    EXPECT_EQ(Point3f::Zero(),               info.target_point);
    EXPECT_EQ(FindRotator("XAxisRotator"),   info.widget);
    EXPECT_EQ("#dd595bff",                   info.color.ToHexString());
    info.guide_direction.Set(0, -1, 0);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kRotation, info.guide_type);
    EXPECT_EQ(Point3f::Zero(),               info.target_point);
    EXPECT_EQ(FindRotator("YAxisRotator"),   info.widget);
    EXPECT_EQ("#4b80e2ff",                   info.color.ToHexString());
    info.guide_direction.Set(0, 0, 1);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kRotation, info.guide_type);
    EXPECT_EQ(Point3f::Zero(),               info.target_point);
    EXPECT_EQ(FindRotator("ZAxisRotator"),   info.widget);
    EXPECT_EQ("#4bc44bff",                   info.color.ToHexString());

    // Otherwise, the free rotator is used.
    info.guide_direction = Normalized(1, 1, -1);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kRotation, info.guide_type);
    EXPECT_EQ(Point3f::Zero(),               info.target_point);
    EXPECT_EQ(FindRotator("FreeRotator"),    info.widget);
    EXPECT_EQ(default_color,                 info.color);
}

TEST_F(RotationToolTest, AxisRotate) {
    CheckNoCommands();

    DragTester dt(FindRotator("YAxisRotator"));
    dt.SetRayDirection(Vector3f(0, -1, 0));
    dt.ApplyMouseDrag(Point3f(0, 0, 1), Point3f(1, 0, 1));

    const auto &cmd = CheckOneCommand<RotateCommand>();
    EXPECT_ROTS_CLOSE(BuildRotation(0, 1, 0, 45), cmd.GetRotation());
    EXPECT_FALSE(cmd.IsInPlace());
    EXPECT_FALSE(cmd.IsAxisAligned());
}

TEST_F(RotationToolTest, FreeRotate) {
    CheckNoCommands();

    // Use grip dragging for this.
    DragTester dt(FindRotator("FreeRotator"));
    context->is_modified_mode = true;
    dt.SetIsModifiedMode(true);  // Turns on in-place flag.
    SetIsAxisAligned(true);
    dt.ApplyGripRotationDrag(Normalized(1, 1, -1),
                             BuildRotation(1, 0, 0, 0),
                             BuildRotation(1, 0, 0, 45));

    const auto &cmd = CheckOneCommand<RotateCommand>();
    EXPECT_ROTS_CLOSE(BuildRotation(1, 0, 0, 45), cmd.GetRotation());
    EXPECT_TRUE(cmd.IsInPlace());
    EXPECT_TRUE(cmd.IsAxisAligned());
}

TEST_F(RotationToolTest, SnapToTarget) {
    CheckNoCommands();

    SetPointTarget(Point3f(0, 0, 0), Vector3f(1, 0, 0));

    DragTester dt(FindRotator("YAxisRotator"));
    dt.ApplyGripRotationDrag(Vector3f(1, 0, 0),
                             BuildRotation(1, 0, 0, 0),
                             BuildRotation(1, 0, 0, 88.8));

    const auto &cmd = CheckOneCommand<RotateCommand>();
    EXPECT_ROTS_CLOSE(BuildRotation(0, 1, 0, 90), cmd.GetRotation());
    EXPECT_FALSE(cmd.IsInPlace());
    EXPECT_FALSE(cmd.IsAxisAligned());
}

TEST_F(RotationToolTest, NoSnapToTarget) {
    CheckNoCommands();

    SetPointTarget(Point3f(0, 0, 0), Vector3f(1, 0, 0));

    DragTester dt(FindRotator("YAxisRotator"));
    dt.ApplyGripRotationDrag(Vector3f(1, 0, 0),
                             BuildRotation(1, 0, 0, 0),
                             BuildRotation(1, 0, 0, 60));

    const auto &cmd = CheckOneCommand<RotateCommand>();
    EXPECT_ROTS_CLOSE(BuildRotation(0, 1, 0, 60), cmd.GetRotation());
    EXPECT_FALSE(cmd.IsInPlace());
    EXPECT_FALSE(cmd.IsAxisAligned());
}
