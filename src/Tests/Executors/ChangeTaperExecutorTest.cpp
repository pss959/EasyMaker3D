#include "Commands/ChangeTaperCommand.h"
#include "Executors/ChangeTaperExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/TaperedModel.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeTaperExecutorTest : public ExecutorTestBase {};

TEST_F(ChangeTaperExecutorTest, TypeName) {
    ChangeTaperExecutor exec;
    EXPECT_EQ("ChangeTaperCommand", exec.GetCommandTypeName());
}

TEST_F(ChangeTaperExecutorTest, ChangeTaper) {
    auto cmd = ParseCommand<ChangeTaperCommand>(
        "ChangeTaperCommand",
        R"(model_names: ["Tapered_0", "Tapered_1"],
           profile_points: [0 1, .6 .5, 1 0], axis: "kZ")");

    ChangeTaperExecutor exec;
    auto context = InitContext(exec);

    // Create and add a couple of TaperedModel instances created from BoxModel
    // instances.
    auto box0  = Model::CreateModel<BoxModel>("Box_0");
    auto box1  = Model::CreateModel<BoxModel>("Box_1");
    auto tapered0 = Model::CreateModel<TaperedModel>("Tapered_0");
    auto tapered1 = Model::CreateModel<TaperedModel>("Tapered_1");
    tapered0->SetOperandModel(box0);
    tapered1->SetOperandModel(box1);
    context.root_model->AddChildModel(tapered0);
    context.root_model->AddChildModel(tapered1);

    context.selection_manager->SelectAll();

    const auto old_taper0 = tapered0->GetTaper();
    const auto old_taper1 = tapered1->GetTaper();

    EXPECT_ENUM_EQ(Model::Status::kPrimary,   tapered0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, tapered1->GetStatus());

    Taper expected;
    expected.axis    = Dim::kZ;
    expected.profile = TaperedModel::CreateProfile(
        Profile::PointVec{ Point2f(0, 1), Point2f(.6f, .5f), Point2f(1, 0) });

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(expected, tapered0->GetTaper());
    EXPECT_EQ(expected, tapered1->GetTaper());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   tapered0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, tapered1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(old_taper0, tapered0->GetTaper());
    EXPECT_EQ(old_taper1, tapered1->GetTaper());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   tapered0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, tapered1->GetStatus());
}
