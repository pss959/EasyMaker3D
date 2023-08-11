#include "Models/BeveledModel.h"
#include "Models/TaperedModel.h"
#include "Models/BoxModel.h"
#include "Models/TextModel.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/Assert.h"
#include "Util/General.h"

/// \ingroup Tests
class ParentModelTest : public SceneTestBase {};

// ----------------------------------------------------------------------------
// Because ParentModel is abstract, these tests use the derived concrete
// BeveledModel and TaperedModel classes. BeveledModel is derived from
// ScaledParentModel and TaperedModel is derived directly from
// ParentModel (and does not have specialized transform syncing).
// ----------------------------------------------------------------------------

TEST_F(ParentModelTest, ChangeOperandModel) {
    auto box0 = Model::CreateModel<BoxModel>();
    auto box1 = Model::CreateModel<BoxModel>();
    box0->SetUniformScale(4);
    box1->SetUniformScale(6);
    box0->SetTranslation(Vector3f(0, 4, 0));
    box1->SetTranslation(Vector3f(0, 6, 0));

    auto beveled = Model::CreateModel<BeveledModel>();
    beveled->SetOperandModel(box0);
    EXPECT_EQ(box0, beveled->GetOperandModel());
    beveled->SetStatus(Model::Status::kUnselected);

    auto bounds = beveled->GetBounds();
    EXPECT_EQ(Vector3f(8, 8, 8), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
    EXPECT_EQ(Vector3f(1, 1, 1), beveled->GetScale());
    EXPECT_EQ(Vector3f(0, 4, 0), beveled->GetTranslation());

    beveled->SetOperandModel(box1);
    bounds = beveled->GetBounds();
    EXPECT_EQ(box1, beveled->GetOperandModel());
    EXPECT_EQ(Vector3f(12, 12, 12), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
    EXPECT_EQ(Vector3f(1, 1, 1), beveled->GetScale());
    EXPECT_EQ(Vector3f(0, 6, 0), beveled->GetTranslation());
}

TEST_F(ParentModelTest, SyncTransforms) {
    auto box = Model::CreateModel<BoxModel>();
    box->SetUniformScale(4);
    box->SetTranslation(Vector3f(0, 4, 0));

    auto rot0 = BuildRotation(0, 1, 0,  30);
    auto rot1 = BuildRotation(1, 0, 0, -40);

    auto tapered = Model::CreateModel<TaperedModel>();
    tapered->SetOperandModel(box);
    tapered->SetRotation(rot0);
    tapered->SetStatus(Model::Status::kUnselected);

    // Transforms should have been synced from the box.
    EXPECT_EQ(Vector3f(4, 4, 4),     tapered->GetScale());
    EXPECT_EQ(Vector3f(0, 4, 0),     tapered->GetTranslation());
    EXPECT_EQ(rot0,                  tapered->GetRotation());
    EXPECT_EQ(Rotationf::Identity(), box->GetRotation());

    // Show the operand model box. The box should have the synced rotation.
    box->SetStatus(Model::Status::kUnselected);
    tapered->SetStatus(Model::Status::kDescendantShown);
    EXPECT_EQ(rot0, tapered->GetRotation());
    EXPECT_EQ(rot0, box->GetRotation());

    // Rotate the box and show the TaperedModel. It should have the new
    // rotation applied.
    box->SetRotation(rot1);
    EXPECT_EQ(rot1, box->GetRotation());

    box->SetStatus(Model::Status::kAncestorShown);
    tapered->SetStatus(Model::Status::kUnselected);
    EXPECT_EQ(rot1, box->GetRotation());
    EXPECT_EQ(rot1, tapered->GetRotation());
}

TEST_F(ParentModelTest, SyncTransformsScaled) {
    auto box = Model::CreateModel<BoxModel>();
    box->SetUniformScale(4);
    box->SetTranslation(Vector3f(0, 4, 0));

    auto rot0 = BuildRotation(0, 1, 0,  30);
    auto rot1 = BuildRotation(1, 0, 0, -40);

    auto beveled = Model::CreateModel<BeveledModel>();
    beveled->SetOperandModel(box);
    beveled->SetRotation(rot0);
    beveled->SetStatus(Model::Status::kUnselected);
    EXPECT_EQ(Vector3f(1, 1, 1),     beveled->GetScale());
    EXPECT_EQ(Vector3f(0, 4, 0),     beveled->GetTranslation());
    EXPECT_EQ(rot0,                  beveled->GetRotation());
    EXPECT_EQ(Rotationf::Identity(), box->GetRotation());

    // Show the operand model box. The box should have the synced rotation.
    box->SetStatus(Model::Status::kUnselected);
    beveled->SetStatus(Model::Status::kDescendantShown);
    EXPECT_EQ(rot0, beveled->GetRotation());
    EXPECT_EQ(rot0, box->GetRotation());

    // Rotate the box and show the BeveledModel. It should have the new
    // rotation applied.
    box->SetRotation(rot1);
    EXPECT_EQ(rot1, box->GetRotation());

    box->SetStatus(Model::Status::kAncestorShown);
    beveled->SetStatus(Model::Status::kUnselected);
    EXPECT_EQ(rot1, box->GetRotation());
    EXPECT_EQ(rot1, beveled->GetRotation());
}

TEST_F(ParentModelTest, Copy) {
    auto box = Model::CreateModel<BoxModel>();
    box->SetUniformScale(4);
    box->SetTranslation(Vector3f(0, 4, 0));

    auto tapered = Model::CreateModel<TaperedModel>();
    tapered->SetOperandModel(box);
    tapered->SetStatus(Model::Status::kUnselected);

    auto copy = tapered->CloneTyped<TaperedModel>(true);
    EXPECT_NOT_NULL(copy->GetOperandModel());
    EXPECT_TRUE(Util::IsA<BoxModel>(copy->GetOperandModel()));
    EXPECT_NE(tapered->GetOperandModel(), copy->GetOperandModel());
    EXPECT_EQ(tapered->GetScale(),       copy->GetScale());
    EXPECT_EQ(tapered->GetRotation(),    copy->GetRotation());
    EXPECT_EQ(tapered->GetTranslation(), copy->GetTranslation());
}

TEST_F(ParentModelTest, CopyScaled) {
    auto box = Model::CreateModel<BoxModel>();
    box->SetUniformScale(4);
    box->SetTranslation(Vector3f(0, 4, 0));

    auto beveled = Model::CreateModel<BeveledModel>();
    beveled->SetOperandModel(box);
    beveled->SetStatus(Model::Status::kUnselected);

    auto copy = beveled->CloneTyped<BeveledModel>(true);
    EXPECT_NOT_NULL(copy->GetOperandModel());
    EXPECT_TRUE(Util::IsA<BoxModel>(copy->GetOperandModel()));
    EXPECT_NE(beveled->GetOperandModel(), copy->GetOperandModel());
    EXPECT_EQ(beveled->GetScale(),       copy->GetScale());
    EXPECT_EQ(beveled->GetRotation(),    copy->GetRotation());
    EXPECT_EQ(beveled->GetTranslation(), copy->GetTranslation());
}

TEST_F(ParentModelTest, Errors) {
    // These test that the ParentModel parent/child functions assert when
    // called directly.
    auto box      = Model::CreateModel<BoxModel>();
    auto tapered = Model::CreateModel<TaperedModel>();
    TEST_THROW(tapered->AddChildModel(box), AssertException,
               "should not be called");
    TEST_THROW(tapered->InsertChildModel(0, box), AssertException,
               "should not be called");
    TEST_THROW(tapered->RemoveChildModel(0), AssertException,
               "should not be called");
    TEST_THROW(tapered->ReplaceChildModel(0, box), AssertException,
               "should not be called");
}
