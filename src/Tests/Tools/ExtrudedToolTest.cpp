#include "Commands/ChangeExtrudedCommand.h"
#include "Models/ExtrudedModel.h"
#include "Models/RootModel.h"
#include "Tools/ExtrudedTool.h"
#include "Tests/Panels/PanelInteractor.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/DraggableWidget.h"

DECL_SHARED_PTR(ExtrudedTool);

/// \ingroup Tests
class ExtrudedToolTest : public ToolTestBase {
  protected:
    ExtrudedToolPtr tool;
    ModelPtr        model;   ///< ExtrudedModel the Tool is attached to.

    /// The constructor sets up an ExtrudedTool attached to a selected
    /// ExtrudedModel.
    ExtrudedToolTest();

    /// The destructor detaches the ExtrudedTool.
    ~ExtrudedToolTest() { tool->DetachFromSelection(); }
};

ExtrudedToolTest::ExtrudedToolTest() {
    // Initialize the ExtrudedTool.
    tool = InitTool<ExtrudedTool>("ExtrudedTool");
    EXPECT_TRUE(tool->IsSpecialized());

    // Create and add an ExtrudedModel.
    model = Model::CreateModel<ExtrudedModel>("Extruded");
    context->root_model->AddChildModel(model);

    // Attach the ExtrudedTool to the ExtrudedModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());

    AddDummyCommandFunction("ChangeExtrudedCommand");
}

TEST_F(ExtrudedToolTest, DragPoint) {
    CheckNoCommands();

    auto sw = SG::FindTypedNodeUnderNode<DraggableWidget>(
        *tool->GetPanel(), "MovablePoint_0");
    DragTester dt(sw);
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.2f, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeExtrudedCommand>();
    EXPECT_EQ(StrVec{ "Extruded" }, cmd.GetModelNames());
    EXPECT_EQ(1,                    cmd.GetProfile().GetPoints()[0][0]);
}

TEST_F(ExtrudedToolTest, SetSides) {
    CheckNoCommands();

    // Change the number of sides using text input and the "SetSides" button.
    PanelInteractor pi(tool->GetPanel());
    auto st = pi.SetTextInput("SidesText", "11");
    pi.ClickButtonPane("SetSides");

    const auto &cmd = CheckOneCommand<ChangeExtrudedCommand>();
    EXPECT_EQ(StrVec{ "Extruded" }, cmd.GetModelNames());
    EXPECT_EQ(11U,                  cmd.GetProfile().GetPointCount());
}
