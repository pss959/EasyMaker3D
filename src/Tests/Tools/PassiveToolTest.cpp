//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tools/PassiveTool.h"
#include "Tests/Panels/PanelInteractor.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"

DECL_SHARED_PTR(PassiveTool);

/// \ingroup Tests
class PassiveToolTest : public ToolTestBase {
  protected:
    PassiveToolPtr tool;
    ModelPtr    model;   ///< Model the Tool is attached to.
    /// The constructor sets up a PassiveTool attached to a selected BoxModel.
    PassiveToolTest();

    /// The destructor detaches the PassiveTool.
    ~PassiveToolTest() { tool->DetachFromSelection(); }
};

PassiveToolTest::PassiveToolTest() {
    // Initialize the PassiveTool.
    tool = InitTool<PassiveTool>("PassiveTool");
    EXPECT_FALSE(tool->IsSpecialized());

    // Create and add a BoxModel.
    model = Model::CreateModel<BoxModel>("Box");
    context->root_model->AddChildModel(model);

    // Attach the PassiveTool to the BoxModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());
}

TEST_F(PassiveToolTest, Attach) {
    // All of the work is done in the constructor, so nothing else to do.
}
