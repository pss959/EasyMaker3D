#include "Commands/ChangeRevSurfCommand.h"
#include "Models/RevSurfModel.h"
#include "Models/RootModel.h"
#include "Place/ClickInfo.h"
#include "Tools/RevSurfTool.h"
#include "Tests/Panels/PanelInteractor.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/DraggableWidget.h"

DECL_SHARED_PTR(RevSurfTool);

/// \ingroup Tests
class RevSurfToolTest : public ToolTestBase {
  protected:
    RevSurfToolPtr tool;
    ModelPtr       model;   ///< RevSurfModel the Tool is attached to.

    /// The constructor sets up an RevSurfTool attached to a selected
    /// RevSurfModel.
    RevSurfToolTest();

    /// The destructor detaches the RevSurfTool.
    ~RevSurfToolTest() { tool->DetachFromSelection(); }
};

RevSurfToolTest::RevSurfToolTest() {
    // Initialize the RevSurfTool.
    tool = InitTool<RevSurfTool>("RevSurfTool");
    EXPECT_TRUE(tool->IsSpecialized());

    // Create and add an RevSurfModel.
    model = Model::CreateModel<RevSurfModel>("RevSurf");
    context->root_model->AddChildModel(model);

    // Attach the RevSurfTool to the RevSurfModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());

    AddDummyCommandFunction("ChangeRevSurfCommand");
}

TEST_F(RevSurfToolTest, AddPoint) {
    CheckNoCommands();

    // Access the area widget and new-point widget.
    auto aw  = SG::FindTypedNodeUnderNode<DraggableWidget>(*tool->GetPanel(),
                                                          "AreaWidget");
    auto npw = SG::FindTypedNodeUnderNode<DraggableWidget>(*tool->GetPanel(),
                                                           "NewPoint");

    // Hover to enable the new-point widget.
    aw->UpdateHoverPoint(Point3f(-.25f, .25f, 0));

    // Click on the new-point widget. This should add a command to create a new
    // profile point.
    ClickInfo info;  // Contents do not matter.
    npw->Click(info);

    const auto &cmd = CheckOneCommand<ChangeRevSurfCommand>();
    EXPECT_EQ(StrVec{ "RevSurf" }, cmd.GetModelNames());
    EXPECT_EQ(4U,                  cmd.GetProfile().GetPointCount());
}

TEST_F(RevSurfToolTest, DragPoint) {
    CheckNoCommands();

    auto sw = SG::FindTypedNodeUnderNode<DraggableWidget>(
        *tool->GetPanel(), "MovablePoint_1");
    DragTester dt(sw);
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.2f, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeRevSurfCommand>();
    EXPECT_EQ(StrVec{ "RevSurf" }, cmd.GetModelNames());
    EXPECT_EQ(3U,                  cmd.GetProfile().GetPointCount());
    EXPECT_EQ(.7f,                 cmd.GetProfile().GetPoints()[1][0]);
}
