//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeTaperCommand.h"
#include "Models/BoxModel.h"
#include "Models/TaperedModel.h"
#include "Models/RootModel.h"
#include "Place/ClickInfo.h"
#include "Tools/TaperTool.h"
#include "Tests/Panels/PanelInteractor.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/DraggableWidget.h"

DECL_SHARED_PTR(TaperTool);

/// \ingroup Tests
class TaperToolTest : public ToolTestBase {
  protected:
    TaperToolPtr    tool;
    TaperedModelPtr model;   ///< TaperedModel the Tool is attached to.

    /// The constructor sets up a TaperTool attached to a selected TaperedModel.
    TaperToolTest();

    /// The destructor detaches the TaperTool.
    ~TaperToolTest() { tool->DetachFromSelection(); }
};

TaperToolTest::TaperToolTest() {
    // Initialize the TaperTool.
    tool = InitTool<TaperTool>("TaperTool");
    EXPECT_TRUE(tool->IsSpecialized());

    // Create and add a TaperedModel.
    auto box = Model::CreateModel<BoxModel>("Box");
    model    = Model::CreateModel<TaperedModel>("Tapered");
    model->SetOperandModel(box);
    context->root_model->AddChildModel(model);

    // Attach the TaperTool to the TaperedModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());

    AddDummyCommandFunction("ChangeTaperCommand");
}

TEST_F(TaperToolTest, AddPoint) {
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

    const auto &cmd = CheckOneCommand<ChangeTaperCommand>();
    EXPECT_EQ(StrVec{ "Tapered" }, cmd.GetModelNames());
    EXPECT_EQ(3U,                  cmd.GetTaper().profile.GetPointCount());
}

TEST_F(TaperToolTest, DragPoint) {
    CheckNoCommands();

    auto sw = SG::FindTypedNodeUnderNode<DraggableWidget>(
        *tool->GetPanel(), "MovablePoint_0");
    DragTester dt(sw);
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.2f, 0, 0));

    const auto &cmd = CheckOneCommand<ChangeTaperCommand>();
    EXPECT_EQ(StrVec{ "Tapered" }, cmd.GetModelNames());
    EXPECT_EQ(2U,                  cmd.GetTaper().profile.GetPointCount());
    EXPECT_CLOSE(.2f,              cmd.GetTaper().profile.GetPoints()[0][0]);
}
