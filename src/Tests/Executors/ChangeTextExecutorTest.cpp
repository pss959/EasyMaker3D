#include "Commands/ChangeTextCommand.h"
#include "Executors/ChangeTextExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/TextModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeTextExecutorTest : public ExecutorTestBase {};

TEST_F(ChangeTextExecutorTest, TypeName) {
    ChangeTextExecutor exec;
    EXPECT_EQ("ChangeTextCommand", exec.GetCommandTypeName());
}

TEST_F(ChangeTextExecutorTest, ChangeText) {
    // Need the real FontSystem and real set-up TextModels for GetMesh() to
    // work.
    UseRealFontSystem(true);

    auto cmd = ParseCommand<ChangeTextCommand>(
        "ChangeTextCommand",
        R"(model_names: ["Text_0", "Text_1"], new_text_string: "B",
           new_font_name: "Arial-Bold", new_char_spacing: 1.5)");

    ChangeTextExecutor exec;
    auto context = InitContext(exec);

    // Create and add two TextModels. Note that they need to have valid text
    // strings before adding them to the RootModel.
    auto text0 = Model::CreateModel<TextModel>("Text_0");
    auto text1 = Model::CreateModel<TextModel>("Text_1");
    text0->SetTextString("0");
    text1->SetTextString("1");
    context.root_model->AddChildModel(text0);
    context.root_model->AddChildModel(text1);

    context.selection_manager->SelectAll();

    const auto old_font0  = text0->GetFontName();
    const auto old_font1  = text1->GetFontName();
    const auto old_space0 = text0->GetCharSpacing();
    const auto old_space1 = text1->GetCharSpacing();

    EXPECT_EQ("0",                            text0->GetTextString());
    EXPECT_EQ("1",                            text1->GetTextString());
    EXPECT_EQ(old_font0,                      text0->GetFontName());
    EXPECT_EQ(old_font1,                      text1->GetFontName());
    EXPECT_EQ(old_space0,                     text0->GetCharSpacing());
    EXPECT_EQ(old_space1,                     text1->GetCharSpacing());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   text0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, text1->GetStatus());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ("B",                            text0->GetTextString());
    EXPECT_EQ("B",                            text1->GetTextString());
    EXPECT_EQ("Arial-Bold",                   text0->GetFontName());
    EXPECT_EQ("Arial-Bold",                   text1->GetFontName());
    EXPECT_EQ(1.5f,                           text0->GetCharSpacing());
    EXPECT_EQ(1.5f,                           text1->GetCharSpacing());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   text0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, text1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ("0",                            text0->GetTextString());
    EXPECT_EQ("1",                            text1->GetTextString());
    EXPECT_EQ(old_font0,                      text0->GetFontName());
    EXPECT_EQ(old_font1,                      text1->GetFontName());
    EXPECT_EQ(old_space0,                     text0->GetCharSpacing());
    EXPECT_EQ(old_space1,                     text1->GetCharSpacing());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   text0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, text1->GetStatus());
}
