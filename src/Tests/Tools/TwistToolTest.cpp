#include <ion/math/vectorutils.h>

#include "Commands/ChangeTwistCommand.h"
#include "Models/TwistedModel.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tools/TwistTool.h"
#include "SG/Search.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/SpinWidget.h"

DECL_SHARED_PTR(TwistTool);

/// \ingroup Tests
class TwistToolTest : public ToolTestBase {
  protected:
    TwistToolPtr     tool;
    TwistedModelPtr  model;   ///< Model the Tool is attached to.
    SpinWidgetPtr    widget;  ///< SpinWidget inside the TwistTool.

    /// The constructor sets up a TwistTool attached to a selected TwistedModel.
    TwistToolTest();

    /// The destructor detaches the TwistTool.
    ~TwistToolTest() { tool->DetachFromSelection(); }
};

TwistToolTest::TwistToolTest() {
    // Initialize the TwistTool.
    tool = InitTool<TwistTool>("TwistTool");
    EXPECT_TRUE(tool->IsSpecialized());

    // Access its SpinWidget.
    widget = SG::FindTypedNodeUnderNode<SpinWidget>(*tool, "SpinWidget");

    // Create and add a TwistedModel with a BoxModel operand.
    model = Model::CreateModel<TwistedModel>("Twisted");
    model->SetOperandModel(Model::CreateModel<BoxModel>("Box"));
    context->root_model->AddChildModel(model);

    // Attach the TwistTool to the TwistedModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());

    AddDummyCommandFunction("ChangeTwistCommand");
}

TEST_F(TwistToolTest, UpdateGripInfo) {
    Grippable::GripInfo info;

    // TwistTool does not change the color.
    const auto default_color = info.color;

    // Close to axis direction uses the axis rotator.
    info.guide_direction.Set(0, 1, 0);
    tool->UpdateGripInfo(info);
    EXPECT_EQ(GripGuideType::kRotation,        info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),         info.target_point);
    EXPECT_EQ(widget->GetSubWidget("Rotator"), info.widget);
    EXPECT_EQ(default_color,                   info.color);

    // Close to perpendicular uses the axis translator.
    info.guide_direction.Set(1, 0, 0);
    tool->UpdateGripInfo(info);
    EXPECT_EQ(GripGuideType::kBasic,              info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),            info.target_point);
    EXPECT_EQ(widget->GetSubWidget("Translator"), info.widget);
    EXPECT_EQ(default_color,                      info.color);

    // Any other direction uses the spin ring.
    info.guide_direction = ion::math::Normalized(Vector3f(1, 1, 1));
    tool->UpdateGripInfo(info);
    EXPECT_EQ(GripGuideType::kRotation,     info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),      info.target_point);
    EXPECT_EQ(widget->GetSubWidget("Ring"), info.widget);
    EXPECT_EQ(default_color,                info.color);
}

TEST_F(TwistToolTest, DragCenter) {
    CheckNoCommands();

    DragTester dt(widget, StrVec{ "Translator" });
    dt.SetRayDirection(-Vector3f::AxisY());
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(1, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeTwistCommand>();
    EXPECT_EQ(Point3f(1, 0, 0), cmd.GetSpin().center);
}

TEST_F(TwistToolTest, DragAxis) {
    CheckNoCommands();

    // Use grip-dragging here; easier to rotate.
    DragTester dt(widget, StrVec{ "Axis", "Rotator" });
    dt.ApplyGripRotationDrag(Vector3f(0, -1, 0),
                             BuildRotation(0, 0, 1,  10),
                             BuildRotation(0, 0, 1, 100));

    const auto &cmd = CheckOneCommand<ChangeTwistCommand>();
    EXPECT_EQ(Vector3f(-1, 0, 0), cmd.GetSpin().axis);
}

TEST_F(TwistToolTest, DragAngle) {
    CheckNoCommands();

    // Use grip-dragging here; easier to rotate.
    DragTester dt(widget, StrVec{ "Ring" });
    dt.ApplyGripRotationDrag(ion::math::Normalized(Vector3f(1, 1, 1)),
                             BuildRotation(1, 0, 0, 0),
                             BuildRotation(1, 0, 0, 45));

    const auto &cmd = CheckOneCommand<ChangeTwistCommand>();
    EXPECT_CLOSE(45, cmd.GetSpin().angle.Degrees());
}

TEST_F(TwistToolTest, DragOffset) {
    CheckNoCommands();

    DragTester dt(widget, StrVec{ "Offset" });
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(0, 1, 0));

    const auto &cmd = CheckOneCommand<ChangeTwistCommand>();
    EXPECT_CLOSE(1, cmd.GetSpin().offset);
}

TEST_F(TwistToolTest, SnapAxisToTarget) {
    CheckNoCommands();

    SetPointTarget(Point3f(0, 0, 0), Vector3f(1, 0, 0));

    // Use grip-dragging here; easier to rotate. Rotate the twist axis around
    // the Z-axis until it is close to the X-axis.
    DragTester dt(widget, StrVec{ "Axis", "Rotator" });
    dt.ApplyGripRotationDrag(Vector3f(0, -1, 0),
                             BuildRotation(0, 0, 1, 10),
                             BuildRotation(0, 0, 1, 98));

    const auto &cmd = CheckOneCommand<ChangeTwistCommand>();
    EXPECT_EQ(Vector3f(1, 0, 0), cmd.GetSpin().axis);
}


TEST_F(TwistToolTest, SnapCenterToTarget) {
    CheckNoCommands();

    SetPointTarget(Point3f(2, 0, 0), Vector3f(0, 1, 0));

    DragTester dt(widget, StrVec{ "Axis", "Translator" });
    dt.SetRayDirection(-Vector3f::AxisY());
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(1.9f, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeTwistCommand>();
    EXPECT_EQ(Point3f(2, 0, 0), cmd.GetSpin().center);
}
