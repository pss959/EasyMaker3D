#include "Commands/ChangeTorusCommand.h"
#include "Models/RootModel.h"
#include "Models/TorusModel.h"
#include "Place/PrecisionStore.h"
#include "SG/Search.h"
#include "Tools/TorusTool.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Util/Tuning.h"
#include "Widgets/ScaleWidget.h"
#include "Widgets/Slider1DWidget.h"

DECL_SHARED_PTR(TorusTool);

/// \ingroup Tests
class TorusToolTest : public ToolTestBase {
  protected:
    TorusToolPtr   tool;
    TorusModelPtr  model;         ///< TorusModel the Tool is attached to.
    ScaleWidgetPtr inner_scaler;  ///< ScaleWidget for the inner radius.
    ScaleWidgetPtr outer_scaler;  ///< ScaleWidget for the outer radius.

    /// The constructor sets up a TorusTool attached to a selected
    /// TorusModel.
    TorusToolTest();

    /// The destructor detaches the TorusTool.
    ~TorusToolTest() { tool->DetachFromSelection(); }
};

TorusToolTest::TorusToolTest() {
    // Initialize the TorusTool.
    tool = InitTool<TorusTool>("TorusTool");
    EXPECT_TRUE(tool->IsSpecialized());

    // Access its ScaleWidgets.
    inner_scaler = SG::FindTypedNodeUnderNode<ScaleWidget>(
        *tool, "InnerRadiusScaler");
    outer_scaler = SG::FindTypedNodeUnderNode<ScaleWidget>(
        *tool, "OuterRadiusScaler");

    // Create and add a TorusModel.
    model = Model::CreateModel<TorusModel>("Torus");
    context->root_model->AddChildModel(model);

    // Set its radii for consistent testing.
    model->SetInnerRadius(1);
    model->SetOuterRadius(4);

    // Attach the TorusTool to the TorusModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());

    // Need a real Executor function to update the radius in the TorusModel for
    // some of the tests to work.
    auto exec_func = [&](const Command &cmd){
        ASSERT(dynamic_cast<const ChangeTorusCommand *>(&cmd));
        const auto &ctc = dynamic_cast<const ChangeTorusCommand &>(cmd);
        if (ctc.IsInnerRadius())
            model->SetInnerRadius(ctc.GetNewRadius());
        else
            model->SetOuterRadius(ctc.GetNewRadius());
    };
    AddCommandFunction("ChangeTorusCommand", exec_func);
}

TEST_F(TorusToolTest, UpdateGripInfo) {
    Grippable::GripInfo info;

    // TorusTool does not change the color.
    const auto default_color = info.color;

    // This should select the min part of the inner scaler.
    info.guide_direction.Set(0, 1, 0);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kBasic,   info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(3, -1, 0),     info.target_point);
    EXPECT_EQ(inner_scaler->GetMinSlider(), info.widget);
    EXPECT_EQ(default_color,                info.color);

    // This should select the max part of the inner scaler.
    info.guide_direction.Set(0, -1, 0);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kBasic,   info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(3, 1, 0),      info.target_point);
    EXPECT_EQ(inner_scaler->GetMaxSlider(), info.widget);
    EXPECT_EQ(default_color,                info.color);

    // This should select the min part of the outer scaler.
    info.guide_direction.Set(1, 0, 0);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kBasic,   info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(-4, 0, 0),     info.target_point);
    EXPECT_EQ(outer_scaler->GetMinSlider(), info.widget);
    EXPECT_EQ(default_color,                info.color);

    // This should select the max part of the outer scaler.
    info.guide_direction.Set(-1, 0, 0);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kBasic,   info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(4, 0, 0),      info.target_point);
    EXPECT_EQ(outer_scaler->GetMaxSlider(), info.widget);
    EXPECT_EQ(default_color,                info.color);
}

TEST_F(TorusToolTest, PointerDrag) {
    // Pointer dragging should add a ChangeTorusCommand.
    CheckNoCommands();

    DragTester dt(outer_scaler, StrVec{ "MaxSlider" });
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.5f, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeTorusCommand>();
    EXPECT_FALSE(cmd.IsInnerRadius());
    EXPECT_EQ(4.5f, cmd.GetNewRadius());
}

TEST_F(TorusToolTest, GripDrag) {
    // Grip dragging should add a ChangeTorusCommand.
    CheckNoCommands();

    DragTester dt(outer_scaler, StrVec{ "MinSlider" });
    dt.ApplyGripDrag(Point3f(0, 0, 0), Point3f(-.1f, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeTorusCommand>();
    EXPECT_FALSE(cmd.IsInnerRadius());
    EXPECT_EQ(12, cmd.GetNewRadius());
}

TEST_F(TorusToolTest, TouchDrag) {
    // Touch dragging should add a ChangeTorusCommand.
    CheckNoCommands();

    DragTester dt(inner_scaler, StrVec{ "MinSlider" });
    dt.ApplyTouchDrag(Point3f(0, 0, 0), Point3f(-1, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeTorusCommand>();
    EXPECT_TRUE(cmd.IsInnerRadius());
    EXPECT_EQ(2, cmd.GetNewRadius());
}

TEST_F(TorusToolTest, SnapRadiusToTarget) {
    CheckNoCommands();

    SetEdgeTargetLength(3);

    DragTester dt(inner_scaler, StrVec{ "MaxSlider" });
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(1.2f, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeTorusCommand>();
    EXPECT_TRUE(cmd.IsInnerRadius());
    EXPECT_EQ(3, cmd.GetNewRadius());
}

TEST_F(TorusToolTest, SnapDiameterToTarget) {
    CheckNoCommands();
    SetEdgeTargetLength(12);

    DragTester dt(outer_scaler, StrVec{ "MaxSlider" });
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(2, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeTorusCommand>();
    EXPECT_FALSE(cmd.IsInnerRadius());
    EXPECT_EQ(6, cmd.GetNewRadius());
}

TEST_F(TorusToolTest, RestoreOuterRadius) {
    CheckNoCommands();

    // Drag the inner radius to make it large enough to change the outer
    // radius, then drag it back and make sure the outer radius is back to
    // where it started.
    EXPECT_EQ(4, model->GetOuterRadius());

    DragTester dt(inner_scaler, StrVec{ "MaxSlider" });
    dt.ApplyMultiMouseDrag(
        std::vector<Point3f>{
            Point3f(0,   0, 0),
            Point3f(6,   0, 0),
            Point3f(.4f, 0, 0),
        });

    const auto &cmd = CheckOneCommand<ChangeTorusCommand>();
    EXPECT_TRUE(cmd.IsInnerRadius());
    EXPECT_EQ(1.5f, cmd.GetNewRadius());
    EXPECT_EQ(4,    model->GetOuterRadius());
}
