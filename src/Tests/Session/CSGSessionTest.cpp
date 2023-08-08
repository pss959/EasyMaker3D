#include "Managers/SceneContext.h"
#include "Models/CSGModel.h"
#include "Models/RootModel.h"
#include "Tests/Session/SessionTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class CSGSessionTest : public SessionTestBase {};

TEST_F(CSGSessionTest, Logo) {
    // Some complex CSG operations.
    LoadSession("Logo");

    const auto &rm = *context.scene_context->root_model;
    EXPECT_EQ(1U, rm.GetChildModelCount());
    const auto cm = std::dynamic_pointer_cast<CSGModel>(rm.GetChildModel(0));
    ASSERT_TRUE(cm);

    ValidateMesh(cm->GetMesh(), "Logo mesh");
}
