#include "App/SceneContext.h"
#include "Commands/CommandList.h"
#include "Managers/CommandManager.h"
#include "Models/RootModel.h"
#include "SessionTests/SessionTestBase.h"
#include "Tests/Testing.h"

TEST_F(SessionTestBase, EmptySessionTest) {
    EXPECT_TRUE(LoadSession("Empty.mvr"));

    EXPECT_TRUE(context.scene_context);
    EXPECT_TRUE(context.scene_context->root_model);
    EXPECT_EQ(0U, context.scene_context->root_model->GetChildModelCount());
    EXPECT_EQ(0U, context.command_manager->GetCommandList()->GetCommandCount());
}
