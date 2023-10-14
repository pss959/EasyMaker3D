#include "Commands/ChangeCSGOperationCommand.h"
#include "Models/BoxModel.h"
#include "Models/CSGModel.h"
#include "Models/RootModel.h"
#include "Tools/CSGTool.h"
#include "Tests/Panels/PanelInteractor.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"

DECL_SHARED_PTR(CSGTool);

/// \ingroup Tests
class CSGToolTest : public ToolTestBase {
  protected:
    CSGToolPtr  tool;
    CSGModelPtr model;   ///< CSGModel the Tool is attached to.

    /// The constructor sets up a CSGTool attached to a selected CSGModel.
    CSGToolTest();

    /// The destructor detaches the CSGTool.
    ~CSGToolTest() { tool->DetachFromSelection(); }
};

CSGToolTest::CSGToolTest() {
    // Initialize the CSGTool.
    tool = InitTool<CSGTool>("CSGTool");
    EXPECT_TRUE(tool->IsSpecialized());

    // Create and add a CSGModel.
    auto box0 = Model::CreateModel<BoxModel>("Box_0");
    auto box1 = Model::CreateModel<BoxModel>("Box_1");
    model     = Model::CreateModel<CSGModel>("CSG");
    model->SetOperation(CSGOperation::kDifference);
    model->SetOperandModels(std::vector<ModelPtr>{ box0, box1 });
    context->root_model->AddChildModel(model);

    // Attach the CSGTool to the CSGModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());

    AddDummyCommandFunction("ChangeCSGOperationCommand");
}

TEST_F(CSGToolTest, ChangeOperation) {
    CheckNoCommands();

    PanelInteractor pi(tool->GetPanel());
    pi.ActivateRadioButtonPane("Intersection");

    const auto &cmd = CheckOneCommand<ChangeCSGOperationCommand>();
    EXPECT_EQ(StrVec{ "CSG" },             cmd.GetModelNames());
    EXPECT_EQ(CSGOperation::kIntersection, cmd.GetNewOperation());
}
