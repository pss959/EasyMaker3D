#include "App/SceneContext.h"
#include "AppTestBase.h"
#include "Commands/CreatePrimitiveModelCommand.h"
#include "Commands/ScaleCommand.h"
#include "Managers/CommandManager.h"
#include "Math/Types.h"
#include "Models/RootModel.h"
#include "Testing.h"

class ScaleCommandTest : public AppTestBase {
};

TEST_F(ScaleCommandTest, CenterSymmetricScale) {
    auto &cmd_mgr = *context.command_manager;

    // Create a Box.
    auto cpc = CreateCommand<CreatePrimitiveModelCommand>();
    cpc->SetType(PrimitiveType::kBox);
    cmd_mgr.AddAndDo(cpc);

    // Verify that it has a default scale and translation.
    const auto &rm = *context.scene_context->root_model;
    EXPECT_EQ(1U, rm.GetChildModelCount());
    const auto &box = *rm.GetChildModel(0);
    EXPECT_EQ(Vector3f(4, 4, 4), box.GetScale());
    EXPECT_EQ(Vector3f(0, 4, 0), box.GetTranslation());

    // Apply a symmetric scale about the center.
    auto sc = CreateCommand<ScaleCommand>();
    sc->SetModelNames(std::vector<std::string>(1, box.GetName()));
    sc->SetMode(ScaleCommand::Mode::kCenterSymmetric);
    sc->SetRatios(Vector3f(2, 3, 4));
    cmd_mgr.AddAndDo(sc);
    EXPECT_EQ(Vector3f(8, 12, 16), box.GetScale());
    EXPECT_EQ(Vector3f(0, 4, 0),   box.GetTranslation());
}

// XXXX Add mode...
