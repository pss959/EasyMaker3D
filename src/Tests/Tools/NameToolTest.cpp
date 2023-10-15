#include "Commands/ChangeNameCommand.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tools/NameTool.h"
#include "Tests/Panels/PanelInteractor.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"

DECL_SHARED_PTR(NameTool);

/// \ingroup Tests
class NameToolTest : public ToolTestBase {
  protected:
    NameToolPtr tool;
    ModelPtr    model;   ///< Model the Tool is attached to.
    /// The constructor sets up a NameTool attached to a selected BoxModel.
    NameToolTest();

    /// The destructor detaches the NameTool.
    ~NameToolTest() { tool->DetachFromSelection(); }
};

NameToolTest::NameToolTest() {
    // Initialize the NameTool.
    tool = InitTool<NameTool>("NameTool");
    EXPECT_FALSE(tool->IsSpecialized());

    // Create and add a BoxModel.
    model = Model::CreateModel<BoxModel>("Box");
    context->root_model->AddChildModel(model);

    // Attach the NameTool to the BoxModel.
    EXPECT_NULL(tool->GetModelAttachedTo());
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);
    EXPECT_NOT_NULL(tool->GetModelAttachedTo());
    EXPECT_EQ(model, tool->GetModelAttachedTo());

    AddDummyCommandFunction("ChangeNameCommand");
}

TEST_F(NameToolTest, Reattach) {
    // Make sure PanelTool::ReattachToSelection() works.
    EXPECT_EQ(model, tool->GetModelAttachedTo());
    tool->ReattachToSelection();
    EXPECT_EQ(model, tool->GetModelAttachedTo());
}

TEST_F(NameToolTest, UpdateGripInfo) {
    // Make sure Tool::GetGrippableNode() works.
    EXPECT_EQ(tool.get(), tool->GetGrippableNode());

    // Make sure Tool::UpdateGripInfo() works; no gripping allowed.
    Grippable::GripInfo info;
    const auto default_color = info.color;
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kNone, info.guide_type);
    EXPECT_EQ(default_color,             info.color);
    EXPECT_NULL(info.widget);
}

TEST_F(NameToolTest, Apply) {
    // Changing the name and activating the Apply button should add a
    // ChangeNameCommand.
    CheckNoCommands();

    PanelInteractor pi(tool->GetPanel());

    auto input = pi.SetTextInput("Input", "Some Name");
    pi.ClickButtonPane("Apply");

    const auto &cmd = CheckOneCommand<ChangeNameCommand>();
    EXPECT_EQ("Box",       cmd.GetModelName());
    EXPECT_EQ("Some Name", cmd.GetNewName());
}

TEST_F(NameToolTest, Update) {
    // Calling Update() on most Tools has no effect.
    tool->Update();
    EXPECT_EQ(model, tool->GetModelAttachedTo());
}
