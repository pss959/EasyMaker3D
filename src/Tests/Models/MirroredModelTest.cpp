#include "Tests/Testing.h"
#include "Models/BoxModel.h"
#include "Models/MirroredModel.h"
#include "Models/CylinderModel.h"
#include "Tests/SceneTestBase.h"

/// \ingroup Tests
class MirroredModelTest : public SceneTestBase {};

TEST_F(MirroredModelTest, DefaultPlane) {
    EXPECT_EQ(Vector3f::AxisX(), MirroredModel::GetDefaultPlaneNormal());

    // 8x8x8 box at (0,4,0).
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetUniformScale(4);
    box->SetTranslation(Vector3f(0, 4, 0));

    MirroredModelPtr mirrored = Model::CreateModel<MirroredModel>();
    mirrored->SetOperandModel(box);

    // The default plane should mirror across X=0 (doing nothing for a box).
    EXPECT_EQ(Vector3f::AxisX(), mirrored->GetPlaneNormal());

    Bounds bounds = mirrored->GetBounds();
    EXPECT_EQ(Vector3f(2, 2, 2), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
    EXPECT_EQ(Vector3f(4, 4, 4), mirrored->GetScale());
    EXPECT_EQ(Vector3f(0, 0, 0), mirrored->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0), mirrored->GetObjectCenterOffset());
    EXPECT_EQ(Vector3f(0, 0, 0), mirrored->GetLocalCenterOffset());
}

TEST_F(MirroredModelTest, OtherPlane) {
    // 8x8x8 box at (0,4,0).
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetUniformScale(4);
    box->SetTranslation(Vector3f(0, 4, 0));

    MirroredModelPtr mirrored = Model::CreateModel<MirroredModel>();
    mirrored->SetOperandModel(box);
    mirrored->SetPlaneNormal(Vector3f::AxisY());
    EXPECT_EQ(Vector3f::AxisY(), mirrored->GetPlaneNormal());

    Bounds bounds = mirrored->GetBounds();
    EXPECT_EQ(Vector3f(2, 2, 2), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
    EXPECT_EQ(Vector3f(4, 4, 4), mirrored->GetScale());
    EXPECT_EQ(Vector3f(0, 0, 0), mirrored->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0), mirrored->GetObjectCenterOffset());
    EXPECT_EQ(Vector3f(0, 0, 0), mirrored->GetLocalCenterOffset());
}

TEST_F(MirroredModelTest, SyncTransforms) {
    auto box = Model::CreateModel<BoxModel>();
    box->SetUniformScale(4);
    box->SetTranslation(Vector3f(0, 4, 0));

    auto rot = BuildRotation(0, 1, 0,  30);

    auto mirrored = Model::CreateModel<MirroredModel>();
    mirrored->SetOperandModel(box);
    mirrored->SetRotation(rot);
    mirrored->SetStatus(Model::Status::kUnselected);

    // Transforms should have been synced from the box, except translation.
    EXPECT_EQ(Vector3f(4, 4, 4),     mirrored->GetScale());
    EXPECT_EQ(Vector3f(0, 0, 0),     mirrored->GetTranslation());
    EXPECT_EQ(rot,                   mirrored->GetRotation());
    EXPECT_EQ(Rotationf::Identity(), box->GetRotation());

    // Show the operand model box. The box should have the synced rotation but
    // retain its translation.
    box->SetStatus(Model::Status::kUnselected);
    mirrored->SetStatus(Model::Status::kDescendantShown);
    EXPECT_EQ(Vector3f(0, 0, 0), mirrored->GetTranslation());
    EXPECT_EQ(Vector3f(0, 4, 0), box->GetTranslation());
    EXPECT_EQ(rot, mirrored->GetRotation());
    EXPECT_EQ(rot, box->GetRotation());
}

TEST_F(MirroredModelTest, IsValid) {
    TEST_THROW(ParseObject<MirroredModel>("MirroredModel {}"),
               Parser::Exception, "No operand model");

    TEST_THROW(ParseObject<MirroredModel>(
                   "MirroredModel { operand_model: BoxModel {},"
                   " plane_normal: 0 0 0 }"),
               Parser::Exception, "Zero-length plane normal");

    // This should not throw.
    auto mirrored = ParseObject<MirroredModel>(
        "MirroredModel { operand_model: BoxModel {} }");
    EXPECT_NOT_NULL(mirrored);
}
