#include "Managers/SceneContext.h"
#include "Models/RootModel.h"
#include "Tests/Sim/SimTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class EmptySimTest_ : public SimTestBase {
  protected:
    virtual void TestResults() override {
        // There shouldn't be any models in the scene.
        const auto &rm = *GetContext().scene_context->root_model;
        EXPECT_EQ(0U, rm.GetChildModelCount());
    }
};

TEST_F(EmptySimTest_, EmptySimTest) {
    RunScript("Empty");
}
