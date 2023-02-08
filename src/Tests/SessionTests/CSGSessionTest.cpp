#include "Managers/SceneContext.h"
#include "Models/CSGModel.h"
#include "Models/RootModel.h"
#include "SessionTests/SessionTestBase.h"
#include "Tests/Testing.h"
#include "Util/General.h"

class CSGSessionTest : public SessionTestBase {
};

TEST_F(CSGSessionTest, Logo) {
    // Some complex CSG operations.
    LoadSession("Logo");

    const auto &rm = *context.scene_context->root_model;
    EXPECT_EQ(1U, rm.GetChildModelCount());
    const auto cm = Util::CastToDerived<CSGModel>(rm.GetChildModel(0));
    ASSERT_TRUE(cm);

    ValidateMesh(cm->GetMesh(), "Logo mesh");
}
