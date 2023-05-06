#include "Managers/SceneContext.h"
#include "Models/RootModel.h"
#include "SimTests/SimTestBase.h"
#include "Tests/Testing.h"

TEST_F(SimTestBase, EmptySimTest) {
    EXPECT_TRUE(RunScript("Empty"));

    // There shouldn't be any models in the scene.
    const auto &rm = *context.scene_context->root_model;
    EXPECT_EQ(0U, rm.GetChildModelCount());
}
