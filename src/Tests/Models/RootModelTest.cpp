#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/General.h"

// ----------------------------------------------------------------------------
// These are used to test RootModel and also the abstract ParentModel class it
// is derived from.
// ----------------------------------------------------------------------------

/// \ingroup Tests
class RootModelTest : public SceneTestBase {};

TEST_F(RootModelTest, Default) {
    auto root = Model::CreateModel<RootModel>();

    EXPECT_EQ(Model::Status::kUnselected, root->GetStatus());

    // Cannot attach targets to the RootModel bounds.
    EXPECT_FALSE(root->CanTargetBounds());

    // The RootModel creates an empty mesh.
    EXPECT_TRUE(root->GetMesh().points.empty());
    EXPECT_TRUE(root->GetMesh().indices.empty());
}

TEST_F(RootModelTest, Children) {
    auto root = Model::CreateModel<RootModel>();
    auto box0 = Model::CreateModel<BoxModel>();
    auto box1 = Model::CreateModel<BoxModel>();
    auto box2 = Model::CreateModel<BoxModel>();

    size_t change_count = 0;
    root->GetTopLevelChanged().AddObserver("key", [&](){ ++change_count; });

    EXPECT_EQ(0U, root->GetChildModelCount());
    EXPECT_NULL(root->GetChildModel(0));

    root->AddChildModel(box0);
    EXPECT_EQ(1U,   change_count);
    EXPECT_EQ(1U,   root->GetChildModelCount());
    EXPECT_EQ(box0, root->GetChildModel(0));
    EXPECT_EQ(0,    root->GetChildModelIndex(box0));
    EXPECT_EQ(-1,   root->GetChildModelIndex(box1));
    EXPECT_NULL(root->GetChildModel(1));

    root->InsertChildModel(0, box1);
    EXPECT_EQ(2U,   change_count);
    EXPECT_EQ(2U,   root->GetChildModelCount());
    EXPECT_EQ(box1, root->GetChildModel(0));
    EXPECT_EQ(box0, root->GetChildModel(1));
    EXPECT_EQ(0,    root->GetChildModelIndex(box1));
    EXPECT_EQ(1,    root->GetChildModelIndex(box0));

    root->ReplaceChildModel(1, box2);
    EXPECT_EQ(4U,   change_count);  // Remove and insert == 2 changes.
    EXPECT_EQ(2U,   root->GetChildModelCount());
    EXPECT_EQ(box1, root->GetChildModel(0));
    EXPECT_EQ(box2, root->GetChildModel(1));
    EXPECT_EQ(0,    root->GetChildModelIndex(box1));
    EXPECT_EQ(1,    root->GetChildModelIndex(box2));

    root->RemoveChildModel(0);
    EXPECT_EQ(5U,   change_count);
    EXPECT_EQ(1U,   root->GetChildModelCount());
    EXPECT_EQ(box2, root->GetChildModel(0));
    EXPECT_EQ(0,    root->GetChildModelIndex(box2));

    root->ClearChildModels();
    EXPECT_EQ(6U,   change_count);
    EXPECT_EQ(0U,   root->GetChildModelCount());
    EXPECT_EQ(-1,   root->GetChildModelIndex(box2));
    EXPECT_NULL(root->GetChildModel(0));

    root->AddChildModel(box0);
    root->Reset();
    EXPECT_EQ(0U, root->GetChildModelCount());
}

TEST_F(RootModelTest, SetLevel) {
    auto root = Model::CreateModel<RootModel>();
    auto box0 = Model::CreateModel<BoxModel>();
    auto box1 = Model::CreateModel<BoxModel>();
    EXPECT_EQ(0, root->GetLevel());
    EXPECT_EQ(0, box0->GetLevel());
    EXPECT_EQ(0, box1->GetLevel());

    root->AddChildModel(box0);
    root->AddChildModel(box1);
    EXPECT_EQ(0, root->GetLevel());
    EXPECT_EQ(1, box0->GetLevel());
    EXPECT_EQ(1, box1->GetLevel());

    root->SetLevel(0);
    EXPECT_EQ(0, root->GetLevel());
    EXPECT_EQ(1, box0->GetLevel());
    EXPECT_EQ(1, box1->GetLevel());

    root->SetLevel(1);
    EXPECT_EQ(1, root->GetLevel());
    EXPECT_EQ(2, box0->GetLevel());
    EXPECT_EQ(2, box1->GetLevel());
}

TEST_F(RootModelTest, SetUse) {
    auto root = Model::CreateModel<RootModel>();
    auto box0 = Model::CreateModel<BoxModel>();
    auto box1 = Model::CreateModel<BoxModel>();
    auto box2 = Model::CreateModel<BoxModel>();
    EXPECT_ENUM_EQ(Model::Use::kNew, root->GetUse());
    EXPECT_ENUM_EQ(Model::Use::kNew, box0->GetUse());
    EXPECT_ENUM_EQ(Model::Use::kNew, box1->GetUse());

    root->AddChildModel(box0);
    root->AddChildModel(box1);
    EXPECT_ENUM_EQ(Model::Use::kNew,     root->GetUse());
    EXPECT_ENUM_EQ(Model::Use::kInScene, box0->GetUse());
    EXPECT_ENUM_EQ(Model::Use::kInScene, box1->GetUse());

    root->SetUse(Model::Use::kInScene);
    EXPECT_ENUM_EQ(Model::Use::kInScene, root->GetUse());
    EXPECT_ENUM_EQ(Model::Use::kInScene, box0->GetUse());
    EXPECT_ENUM_EQ(Model::Use::kInScene, box1->GetUse());

    root->AddChildModel(box2);
    EXPECT_ENUM_EQ(Model::Use::kInScene, box2->GetUse());
}

TEST_F(RootModelTest, Clone) {
    auto root = Model::CreateModel<RootModel>();
    auto box0 = Model::CreateModel<BoxModel>();
    auto box1 = Model::CreateModel<BoxModel>();
    root->AddChildModel(box0);
    root->AddChildModel(box1);

    auto copy = root->CloneTyped<RootModel>(true);
    EXPECT_EQ(2U,   copy->GetChildModelCount());
    EXPECT_TRUE(Util::IsA<BoxModel>(copy->GetChildModel(0)));
    EXPECT_TRUE(Util::IsA<BoxModel>(copy->GetChildModel(1)));
    EXPECT_NE(box0, copy->GetChildModel(0));
    EXPECT_NE(box1, copy->GetChildModel(1));
}

TEST_F(RootModelTest, SetModelVisibility) {
    auto root = Model::CreateModel<RootModel>();
    auto box0 = Model::CreateModel<BoxModel>();
    auto box1 = Model::CreateModel<BoxModel>();

    EXPECT_EQ(0U, root->GetHiddenModelCount());
    root->AddChildModel(box0);
    root->AddChildModel(box1);
    EXPECT_EQ(0U, root->GetHiddenModelCount());

    // Hiding/showing causes notification.
    size_t change_count = 0;
    root->GetTopLevelChanged().AddObserver("key", [&](){ ++change_count; });

    root->SetModelVisibility(box0, false);
    EXPECT_EQ(1U, change_count);
    EXPECT_EQ(1U, root->GetHiddenModelCount());
    EXPECT_ENUM_EQ(Model::Status::kHiddenByUser, box0->GetStatus());

    root->SetModelVisibility(box1, false);
    EXPECT_EQ(2U, change_count);
    EXPECT_EQ(2U, root->GetHiddenModelCount());
    EXPECT_ENUM_EQ(Model::Status::kHiddenByUser, box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kHiddenByUser, box1->GetStatus());

    root->SetModelVisibility(box0, true);
    EXPECT_EQ(3U, change_count);
    EXPECT_EQ(1U, root->GetHiddenModelCount());
    EXPECT_ENUM_EQ(Model::Status::kUnselected,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kHiddenByUser, box1->GetStatus());
}

TEST_F(RootModelTest, Uniforms) {
    // Need to have a real ModelRoot node with programs set up.
    auto root = ReadRealNode<RootModel>(
        R"(children: [<"nodes/ModelRoot.emd">])", "ModelRoot");

    EXPECT_FALSE(root->AreEdgesShown());
    root->ShowEdges(true);
    EXPECT_TRUE(root->AreEdgesShown());

    root->UpdateGlobalUniforms(Matrix4f::Identity(), Vector3f(20, 30, 40));
}
