#include "Commands/ChangeComplexityCommand.h"
#include "Models/SphereModel.h"
#include "Models/RootModel.h"
#include "SG/Search.h"
#include "Tools/ComplexityTool.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Util/Tuning.h"
#include "Widgets/DraggableWidget.h"

DECL_SHARED_PTR(ComplexityTool);

/// \ingroup Tests
class ComplexityToolTest : public ToolTestBase {
  protected:
    ComplexityToolPtr  tool;
    ModelPtr           model;   ///< Model the Tool is attached to.
    DraggableWidgetPtr widget;  ///< DraggableWidget inside the ComplexityTool.

    /// The constructor sets up a ComplexityTool attached to a selected
    /// SphereModel.
    ComplexityToolTest();

    /// The destructor detaches the ComplexityTool.
    ~ComplexityToolTest() { tool->DetachFromSelection(); }
};

ComplexityToolTest::ComplexityToolTest() {
    // Initialize the ComplexityTool.
    tool = InitTool<ComplexityTool>("ComplexityTool");
    EXPECT_FALSE(tool->IsSpecialized());

    // Access its DraggableWidget.
    widget = SG::FindTypedNodeUnderNode<DraggableWidget>(*tool, "Slider");

    // Create and add a SphereModel.
    model = Model::CreateModel<SphereModel>("Sphere");
    context->root_model->AddChildModel(model);

    // Set its complexity for consistent testing.
    model->SetComplexity(.2f);

    // Attach the ComplexityTool to the SphereModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());

    AddDummyCommandFunction("ChangeComplexityCommand");
}

TEST_F(ComplexityToolTest, UpdateGripInfo) {
    Grippable::GripInfo info;

    // ComplexityTool does not change the color.
    const auto default_color = info.color;

    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kBasic,  info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(-4.8f, 4, 0), info.target_point);
    EXPECT_EQ(widget,                      info.widget);
    EXPECT_EQ(default_color,               info.color);
}

TEST_F(ComplexityToolTest, PointerDrag) {
    // Pointer dragging should add a ChangeComplexityCommand.
    CheckNoCommands();

    DragTester dt(widget);
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.8f, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeComplexityCommand>();
    EXPECT_EQ(StrVec{ "Sphere" }, cmd.GetModelNames());
    EXPECT_EQ(.25f,               cmd.GetNewComplexity());
}

TEST_F(ComplexityToolTest, GripDrag) {
    // Grip dragging should add a ChangeComplexityCommand.
    CheckNoCommands();

    DragTester dt(widget);
    dt.ApplyGripDrag(Point3f(0, 0, 0), Point3f(1, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeComplexityCommand>();
    EXPECT_EQ(StrVec{ "Sphere" }, cmd.GetModelNames());
    EXPECT_EQ(1,                  cmd.GetNewComplexity());
}

TEST_F(ComplexityToolTest, TouchDrag) {
    // Touch dragging should add a ChangeComplexityCommand.
    CheckNoCommands();

    DragTester dt(widget);
    dt.ApplyTouchDrag(Point3f(0, 0, 0), Point3f(1, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeComplexityCommand>();
    EXPECT_EQ(StrVec{ "Sphere" }, cmd.GetModelNames());
    EXPECT_CLOSE(.2625f,          cmd.GetNewComplexity());
}

TEST_F(ComplexityToolTest, FaceCamera) {
    // Faces along +Z by default.
    EXPECT_EQ(Rotationf::Identity(), tool->GetRotation());

    // Change the rotation in the parent; the ColorTool should rotate to
    // compensate.
    tool_parent->SetRotation(BuildRotation(0, 1, 0, 30));
    tool->Update();
    EXPECT_EQ(BuildRotation(0, 1, 0, -30), tool->GetRotation());
}
