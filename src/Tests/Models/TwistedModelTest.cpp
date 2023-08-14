#include "Math/Linear.h"
#include "Math/MeshValidation.h"
#include "Math/Spin.h"
#include "Models/BoxModel.h"
#include "Models/TwistedModel.h"
#include "Tests/Testing.h"
#include "Tests/SceneTestBase.h"

/// \ingroup Tests
class TwistedModelTest : public SceneTestBase {};

TEST_F(TwistedModelTest, DefaultTwist) {
    // 2x8x2 box at (0,4,0).
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(2, 8, 2));
    box->SetTranslation(Vector3f(0, 4, 0));

    TwistedModelPtr twisted = Model::CreateModel<TwistedModel>();
    twisted->SetComplexity(.158f);  // 4 slices.
    twisted->SetOperandModel(box);

    // Should be using a default twist.
    Spin default_spin;
    EXPECT_EQ(default_spin, twisted->GetSpin());

    // The result should be the original box, but sliced.
    const auto &mesh = twisted->GetMesh();
    EXPECT_EQ(32U, mesh.points.size());
    EXPECT_EQ(60U, mesh.GetTriangleCount());

    // Bounds should not have changed.
    const Bounds bounds = twisted->GetScaledBounds();
    EXPECT_EQ(Point3f::Zero(),    bounds.GetCenter());
    EXPECT_EQ(Vector3f(4, 16, 4), bounds.GetSize());

    // Translation should not have changed and the offset should be 0.
    EXPECT_EQ(Vector3f(0, 4, 0), twisted->GetTranslation());
    EXPECT_EQ(Vector3f::Zero(),  twisted->GetObjectCenterOffset());
}

TEST_F(TwistedModelTest, IsValid) {
    TestInvalid("TwistedModel {}", "No operand model specified");
    TestInvalid("TwistedModel { operand_model: BoxModel {}, axis: 0 0 0 }",
                "Zero-length spin axis");
    TestValid("TwistedModel { operand_model: BoxModel {}, axis: 0 1 0 }");
}

TEST_F(TwistedModelTest, Twist90) {
    // 2x8x2 box at (0,4,0).
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(2, 8, 2));
    box->SetTranslation(Vector3f(0, 4, 0));

    // Twist 90 degrees clockwise.
    Spin spin;
    spin.angle = Anglef::FromDegrees(-90);

    TwistedModelPtr twisted = Model::CreateModel<TwistedModel>();
    twisted->SetComplexity(.158f);  // 4 slices.
    twisted->SetSpin(spin);
    twisted->SetOperandModel(box);

    EXPECT_EQ(spin, twisted->GetSpin());

    // Should be the same number of points and triangles as the untwisted case.
    const auto &mesh = twisted->GetMesh();
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
    EXPECT_EQ(32U, mesh.points.size());
    EXPECT_EQ(60U, mesh.GetTriangleCount());

    const Bounds bounds = twisted->GetScaledBounds();
    EXPECT_EQ(Point3f::Zero(), bounds.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(5.65686f, 16, 5.65686f), bounds.GetSize());

    // Translation and offset should not have changed.
    EXPECT_EQ(Vector3f(0, 4, 0), twisted->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0), twisted->GetObjectCenterOffset());
}

TEST_F(TwistedModelTest, Twist90Offset) {
    // 2x8x2 box at (0,4,0).
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(2, 8, 2));
    box->SetTranslation(Vector3f(0, 4, 0));

    // Twist 90 degrees clockwise around an off-center axis.
    Spin spin;
    spin.center.Set(1, 0, 0);
    spin.angle = Anglef::FromDegrees(-90);

    TwistedModelPtr twisted = Model::CreateModel<TwistedModel>();
    twisted->SetComplexity(.158f);  // 4 slices.
    twisted->SetSpin(spin);
    twisted->SetOperandModel(box);

    EXPECT_EQ(spin, twisted->GetSpin());

    // Should be the same number of points and triangles as the untwisted case.
    const auto &mesh = twisted->GetMesh();
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
    EXPECT_EQ(32U, mesh.points.size());
    EXPECT_EQ(60U, mesh.GetTriangleCount());

    const Bounds bounds = twisted->GetScaledBounds();
    EXPECT_EQ(Point3f::Zero(), bounds.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(5.76744f, 16, 5.76744f), bounds.GetSize());

    // Translation should not have changed, but offset has to compensate for
    // the off-center twist.
    EXPECT_EQ(Vector3f(0, 4, 0), twisted->GetTranslation());
    EXPECT_VECS_CLOSE(Vector3f(0.3467f, 0, -0.6533f),
                      twisted->GetObjectCenterOffset());
}

TEST_F(TwistedModelTest, TwistXAxis) {
    // Similar to Twist90 test, but uses X axis.

    // 8x2x2 box at (0,1,0).
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(8, 2, 2));
    box->SetTranslation(Vector3f(0, 1, 0));

    // Twist 90 degrees counterclockwise around X axis.
    Spin spin;
    spin.axis.Set(1, 0, 0);
    spin.angle = Anglef::FromDegrees(-90);

    TwistedModelPtr twisted = Model::CreateModel<TwistedModel>();
    twisted->SetComplexity(.158f);  // 4 slices.
    twisted->SetSpin(spin);
    twisted->SetOperandModel(box);

    EXPECT_EQ(spin, twisted->GetSpin());

    const auto &mesh = twisted->GetMesh();
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
    EXPECT_EQ(32U, mesh.points.size());
    EXPECT_EQ(60U, mesh.GetTriangleCount());

    const Bounds bounds = twisted->GetScaledBounds();
    EXPECT_EQ(Point3f::Zero(), bounds.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(16, 5.65686f, 5.65686f), bounds.GetSize());

    // Translation and offset should not have changed.
    EXPECT_EQ(Vector3f(0, 1, 0), twisted->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0), twisted->GetObjectCenterOffset());
}
