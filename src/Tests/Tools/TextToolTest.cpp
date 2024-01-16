#include "Commands/ChangeTextCommand.h"
#include "Models/TextModel.h"
#include "Models/RootModel.h"
#include "Tools/TextTool.h"
#include "Tests/Panels/PanelInteractor.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/DraggableWidget.h"

DECL_SHARED_PTR(TextTool);

/// \ingroup Tests
class TextToolTest : public ToolTestBase {
  protected:
    TextToolPtr  tool;
    TextModelPtr model;   ///< TextModel the Tool is attached to.

    /// The constructor sets up an TextTool attached to a selected
    /// TextModel.
    TextToolTest();

    /// The destructor detaches the TextTool.
    ~TextToolTest() { tool->DetachFromSelection(); }
};

TextToolTest::TextToolTest() {
    // Initialize the TextTool.
    tool = InitTool<TextTool>("TextTool");
    EXPECT_TRUE(tool->IsSpecialized());

    // Create a TextModel and set its text string so it is valid before adding
    // it.
    model = Model::CreateModel<TextModel>("Text");
    model->SetTextString("X");
    context->root_model->AddChildModel(model);

    // Attach the TextTool to the TextModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());

    AddDummyCommandFunction("ChangeTextCommand");
}

TEST_F(TextToolTest, ChangeTextAndFont) {
    CheckNoCommands();

    PanelInteractor pi(tool->GetPanel());
    pi.SetTextInput("TextInput", "Blah");
    pi.ChangeDropdownChoice("Font", "Verdana-Regular");
    CheckNoCommands();

    pi.ClickButtonPane("Apply");

    const auto &cmd = CheckOneCommand<ChangeTextCommand>();
    EXPECT_EQ(StrVec{ "Text" },  cmd.GetModelNames());
    EXPECT_EQ("Blah",            cmd.GetNewTextString());
    EXPECT_EQ("Verdana-Regular", cmd.GetNewFontName());
}

TEST_F(TextToolTest, ChangeSpacing) {
    CheckNoCommands();

    PanelInteractor pi(tool->GetPanel());
    pi.DragSlider("Spacing", Vector2f(.4f, 0));
    CheckNoCommands();

    pi.ClickButtonPane("Apply");

    const auto &cmd = CheckOneCommand<ChangeTextCommand>();
    EXPECT_EQ(StrVec{ "Text" }, cmd.GetModelNames());
    EXPECT_CLOSE(2.6f,          cmd.GetNewCharSpacing());
}
