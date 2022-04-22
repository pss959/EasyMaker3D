#include <vector>

#include "Testing.h"
#include "Managers/ClipboardManager.h"
#include "Models/BoxModel.h"
#include "Models/CylinderModel.h"
#include "SceneTestBase.h"

class ClipboardTest : public SceneTestBase {
  protected:
    /// Vector to use for passing Models to ClipboardManager.
    std::vector<ModelPtr> models;
};

TEST_F(ClipboardTest, CopyOne) {
    ClipboardManager mgr;
    EXPECT_TRUE(mgr.Get().empty());

    ModelPtr box = Model::CreateModel<BoxModel>();
    models.push_back(box);

    mgr.StoreCopies(models);
    EXPECT_EQ(1U, mgr.Get().size());
    EXPECT_EQ("Box_1A", mgr.Get()[0]->GetName());
}
