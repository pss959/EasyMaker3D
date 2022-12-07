#include <vector>

#include "Tests/Testing.h"
#include "Managers/ClipboardManager.h"
#include "Models/BoxModel.h"
#include "Models/ClippedModel.h"
#include "Models/CylinderModel.h"
#include "Tests/SceneTestBase.h"
#include "Util/General.h"

class ClipboardTest : public SceneTestBase {
  protected:
    ClipboardManager mgr;

    /// Vector to use for passing Models to ClipboardManager.
    std::vector<ModelPtr> models;
};

TEST_F(ClipboardTest, CopyOne) {
    EXPECT_TRUE(mgr.Get().empty());

    ModelPtr box = Model::CreateModel<BoxModel>("ABox");
    models.push_back(box);
    EXPECT_EQ(1U, box->GetShapes().size());

    mgr.StoreCopies(models);
    EXPECT_EQ(1U, mgr.Get().size());
    const auto &copy = mgr.Get()[0];
    EXPECT_TRUE(Util::IsA<BoxModel>(copy));
    EXPECT_NE(copy, box);
    EXPECT_EQ("ABox", copy->GetName());
    EXPECT_EQ(1U, copy->GetShapes().size());

    // Meshes should have the same contents.
    EXPECT_EQ(box->GetMesh().points.size(),  copy->GetMesh().points.size());
    EXPECT_EQ(box->GetMesh().indices.size(), copy->GetMesh().indices.size());

    const std::vector<ModelPtr> clones = mgr.CreateClones();
    EXPECT_EQ(1U, clones.size());
    EXPECT_TRUE(Util::IsA<BoxModel>(clones[0]));
    EXPECT_NE(clones[0], box);
    EXPECT_NE(clones[0], copy);
    EXPECT_EQ("ABox", clones[0]->GetName());
    EXPECT_EQ(1U, clones[0]->GetShapes().size());
}

TEST_F(ClipboardTest, CopyTwo) {
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

TEST_F(ClipboardTest, CopyParent) {
    ModelPtr        box     = Model::CreateModel<BoxModel>("ABox");
    ClippedModelPtr clipped = Model::CreateModel<ClippedModel>("AClip");
    clipped->SetOriginalModel(box);
    clipped->AddPlane(Plane(0, Vector3f::AxisY()));
    clipped->SetStatus(Model::Status::kUnselected);
    EXPECT_EQ(1U, box->GetShapes().size());
    EXPECT_EQ(1U, clipped->GetShapes().size());

    models.push_back(clipped);
    mgr.StoreCopies(models);

    EXPECT_EQ(1U, mgr.Get().size());
    const auto &copy = mgr.Get()[0];
    EXPECT_TRUE(Util::IsA<ClippedModel>(copy));
    EXPECT_NE(copy, clipped);
    EXPECT_EQ("AClip", copy->GetName());
    EXPECT_EQ(1U, copy->GetShapes().size());

    // Child should be present.
    const auto ccopy = Util::CastToDerived<ClippedModel>(copy);
    EXPECT_TRUE(clipped->GetOriginalModel());
    EXPECT_TRUE(ccopy->GetOriginalModel());
    EXPECT_TRUE(Util::IsA<BoxModel>(clipped->GetOriginalModel()));
    EXPECT_TRUE(Util::IsA<BoxModel>(ccopy->GetOriginalModel()));
    EXPECT_NE(clipped->GetOriginalModel(), ccopy->GetOriginalModel());
}

TEST_F(ClipboardTest, CopyModified) {
    EXPECT_TRUE(mgr.Get().empty());

    auto cyl = Model::CreateModel<CylinderModel>("ACyl");
    cyl->SetTopRadius(2);
    models.push_back(cyl);

    mgr.StoreCopies(models);
    EXPECT_EQ(1U, mgr.Get().size());
    auto cyl_copy = Util::CastToDerived<CylinderModel>(mgr.Get()[0]);
    EXPECT_NOT_NULL(cyl_copy.get());
    EXPECT_EQ(2, cyl_copy->GetTopRadius());

    const std::vector<ModelPtr> clones = mgr.CreateClones();
    EXPECT_EQ(1U, clones.size());
    auto cyl_clone = Util::CastToDerived<CylinderModel>(clones[0]);
    EXPECT_NOT_NULL(cyl_clone.get());
    EXPECT_EQ(2, cyl_clone->GetTopRadius());
}
