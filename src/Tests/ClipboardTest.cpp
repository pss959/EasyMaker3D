#include <vector>

#include "Testing.h"
#include "Managers/ClipboardManager.h"
#include "Models/BoxModel.h"
#include "Models/CylinderModel.h"
#include "SceneTestBase.h"
#include "Util/General.h"

class ClipboardTest : public SceneTestBase {
  protected:
    /// Vector to use for passing Models to ClipboardManager.
    std::vector<ModelPtr> models;
};

TEST_F(ClipboardTest, CopyOne) {
    ClipboardManager mgr;
    EXPECT_TRUE(mgr.Get().empty());

    ModelPtr box = Model::CreateModel<BoxModel>("ABox");
    models.push_back(box);

    mgr.StoreCopies(models);
    EXPECT_EQ(1U, mgr.Get().size());
    const auto &copy = mgr.Get()[0];
    EXPECT_TRUE(Util::IsA<BoxModel>(copy));
    EXPECT_NE(copy, box);
    EXPECT_EQ("ABox", copy->GetName());

    // Meshes should have the same contents.
    EXPECT_EQ(box->GetMesh().points.size(),  copy->GetMesh().points.size());
    EXPECT_EQ(box->GetMesh().indices.size(), copy->GetMesh().indices.size());
}

TEST_F(ClipboardTest, CopyTwo) {
    ClipboardManager mgr;
    EXPECT_TRUE(mgr.Get().empty());

    auto box = Model::CreateModel<BoxModel>("ABox");
    auto cyl = Model::CreateModel<CylinderModel>("ACyl");
    models.push_back(box);
    models.push_back(cyl);

    mgr.StoreCopies(models);
    EXPECT_EQ(2U, mgr.Get().size());
    const auto &bcopy = mgr.Get()[0];
    const auto &ccopy = mgr.Get()[1];
    EXPECT_TRUE(Util::IsA<BoxModel>(bcopy));
    EXPECT_TRUE(Util::IsA<CylinderModel>(ccopy));
    EXPECT_NE(bcopy, box);
    EXPECT_NE(ccopy, cyl);
    EXPECT_EQ("ABox", bcopy->GetName());
    EXPECT_EQ("ACyl", ccopy->GetName());
}
