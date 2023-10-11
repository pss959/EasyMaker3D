#include "Commands/ChangeColorCommand.h"
#include "Math/ColorRing.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Place/ClickInfo.h"
#include "SG/Search.h"
#include "Tools/ColorTool.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Util/Tuning.h"
#include "Widgets/DraggableWidget.h"

DECL_SHARED_PTR(ColorTool);

/// \ingroup Tests
class ColorToolTest : public ToolTestBase {
  protected:
    ColorToolPtr       tool;
    ModelPtr           model;   ///< Model the Tool is attached to.
    DraggableWidgetPtr widget;  ///< DraggableWidget inside the ColorTool.

    /// The constructor sets up a ColorTool attached to a selected BoxModel.
    ColorToolTest();

    /// The destructor detaches the ColorTool.
    ~ColorToolTest() { tool->DetachFromSelection(); }
};

ColorToolTest::ColorToolTest() {
    // Initialize the ColorTool.
    tool = InitTool<ColorTool>("ColorTool");
    EXPECT_FALSE(tool->IsSpecialized());

    // Access its DraggableWidget.
    widget = SG::FindTypedNodeUnderNode<DraggableWidget>(*tool, "Widget");

    // Create and add a BoxModel.
    model = Model::CreateModel<BoxModel>("Box");
    context->root_model->AddChildModel(model);

    // Set its color for consistent testing.
    model->SetColor(ColorRing::GetColorForPoint(Point2f(.8f, 0)));

    // Attach the ColorTool to the BoxModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());

    AddDummyCommandFunction("ChangeColorCommand");
}

TEST_F(ColorToolTest, UpdateGripInfo) {
    Grippable::GripInfo info;
    tool->UpdateGripInfo(info);
    EXPECT_EQ(GripGuideType::kBasic,        info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(3.2f, 7, .1f), info.target_point);
    EXPECT_EQ(widget,                       info.widget);
    EXPECT_EQ(model->GetColor(),            info.color);
}

TEST_F(ColorToolTest, Click) {
    // Clicking should add a ChangeColorCommand.
    CheckNoCommands();

    ClickInfo info;
    info.hit.point.Set(.8f, .8f, .1f);
    widget->Click(info);

    const auto &cmd = CheckOneCommand<ChangeColorCommand>();
    EXPECT_EQ(ColorRing::GetColorForPoint(Point2f(.8f, .8f)),
              cmd.GetNewColor());
}

TEST_F(ColorToolTest, PointerDrag) {
    // Pointer dragging should add a ChangeColorCommand.
    CheckNoCommands();

    DragTester dt(widget);
    dt.ApplyMouseDrag(Point3f(0, 0, .1f), Point3f(.8f, .8f, .1f));

    const auto &cmd = CheckOneCommand<ChangeColorCommand>();
    EXPECT_EQ(ColorRing::GetColorForPoint(Point2f(.8f, .8f)),
              cmd.GetNewColor());

    // Drag off the Widget to make sure it works.
    dt.ApplyMouseDrag(Point3f(.8f, .8f, .1f), Point3f(2, .8f, .1f), 0, true);
}

TEST_F(ColorToolTest, GripDrag) {
    // Grip dragging should add a ChangeColorCommand.
    CheckNoCommands();

    // Grip dragging scales by this, do use the inverse. Also note that grip
    // dragging uses relative motion, so the new color is relative to the
    // starting color.
    const float s = 1 / TK::kColorToolGripDragScale;
    DragTester dt(widget);
    dt.ApplyGripDrag(Point3f(0, 0, .1f), Point3f(s * -1.6f, s * -.8f, .1f));

    const auto &cmd = CheckOneCommand<ChangeColorCommand>();
    EXPECT_EQ(ColorRing::GetColorForPoint(Point2f(-.8f, -.8f)),
              cmd.GetNewColor());
}

TEST_F(ColorToolTest, TouchDrag) {
    // Touch dragging should NOT add any commands.
    CheckNoCommands();
    DragTester dt(widget);
    dt.ApplyTouchDrag(Point3f(0, 0, .1f), Point3f(.5f, .6f, .1f));
    CheckNoCommands();
}
