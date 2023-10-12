#include "Commands/ChangeBevelCommand.h"
#include "Models/BoxModel.h"
#include "Models/BeveledModel.h"
#include "Models/RootModel.h"
#include "Place/ClickInfo.h"
#include "Tools/BevelTool.h"
#include "Tests/Panels/PanelInteractor.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"
#include "Widgets/DraggableWidget.h"

DECL_SHARED_PTR(BevelTool);

/// \ingroup Tests
class BevelToolTest : public ToolTestBase {
  protected:
    BevelToolPtr    tool;
    BeveledModelPtr model;   ///< BeveledModel the Tool is attached to.

    /// The constructor sets up a BevelTool attached to a selected BeveledModel.
    BevelToolTest();

    /// The destructor detaches the BevelTool.
    ~BevelToolTest() { tool->DetachFromSelection(); }
};

BevelToolTest::BevelToolTest() {
    // Initialize the BevelTool.
    tool = InitTool<BevelTool>("BevelTool");
    EXPECT_TRUE(tool->IsSpecialized());

    // Create and add a BeveledModel.
    auto box = Model::CreateModel<BoxModel>("Box");
    model    = Model::CreateModel<BeveledModel>("Bevel");
    model->SetOperandModel(box);
    context->root_model->AddChildModel(model);

    // Attach the BevelTool to the BeveledModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());

    AddDummyCommandFunction("ChangeBevelCommand");
}

TEST_F(BevelToolTest, ChangeScale) {
    CheckNoCommands();

    // Drag the scale slider to create a change.
    PanelInteractor pi(tool->GetPanel());
    pi.DragSlider("ScaleSlider", Vector2f(.5f, 0));

    const auto &cmd = CheckOneCommand<ChangeBevelCommand>();
    EXPECT_CLOSE(3.21f, cmd.GetBevel().scale);
}

TEST_F(BevelToolTest, AddPoint) {
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

    const auto &cmd = CheckOneCommand<ChangeBevelCommand>();
    EXPECT_EQ(3U, cmd.GetBevel().profile.GetPointCount());
}
