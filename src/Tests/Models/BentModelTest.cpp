#include "Math/Linear.h"
#include "Math/MeshValidation.h"
#include "Math/Spin.h"
#include "Models/BoxModel.h"
#include "Models/BentModel.h"
#include "Models/CylinderModel.h"
#include "Tests/Testing.h"
#include "Tests/SceneTestBase.h"

/// \ingroup Tests
class BentModelTest : public SceneTestBase {};

TEST_F(BentModelTest, DefaultBend) {
    // 10x2x2 box at (0,1,0).
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(10, 2, 2));
    box->SetTranslation(Vector3f(0, 1, 0));

    BentModelPtr bent = Model::CreateModel<BentModel>();
    bent->SetComplexity(.09f);
    bent->SetOperandModel(box);

    // Should be using a default bend.
    Spin default_spin;
    EXPECT_EQ(default_spin, bent->GetSpin());

    // The result should be the original box, but sliced.
    const auto &mesh = bent->GetMesh();
    EXPECT_EQ(48U, mesh.points.size());
    EXPECT_EQ(92U, mesh.GetTriangleCount());

    // Bounds should not have changed.
    const Bounds bounds = bent->GetScaledBounds();
    EXPECT_EQ(Point3f::Zero(),    bounds.GetCenter());
    EXPECT_EQ(Vector3f(20, 4, 4), bounds.GetSize());

    // Translation should not have changed and the offset should be 0.
    EXPECT_EQ(Vector3f(0, 1, 0), bent->GetTranslation());
    EXPECT_EQ(Vector3f::Zero(),  bent->GetObjectCenterOffset());
}

TEST_F(BentModelTest, IsValid) {
    SetParseTypeName("BentModel");
    TestInvalid("", "No operand model specified");
    TestInvalid("operand_model: BoxModel {}, axis: 0 0 0 }",
                "Zero-length spin axis");
    TestValid("operand_model: BoxModel {}, axis: 0 1 0");
}

TEST_F(BentModelTest, Bend90) {
    // 10x2x2 box at (0,1,0).
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(10, 2, 2));
    box->SetTranslation(Vector3f(0, 1, 0));

    // Bend 90 degrees clockwise around +Y with the center at the left end.
    // This should create a 90-degree curve ending near Z=5.
    Spin spin;
    spin.center.Set(-5, 0, 0);
    spin.angle = Anglef::FromDegrees(-90);

    BentModelPtr bent = Model::CreateModel<BentModel>();
    bent->SetComplexity(.09f);
    bent->SetSpin(spin);
    bent->SetOperandModel(box);

    EXPECT_EQ(spin, bent->GetSpin());

    // Should be the same number of points and triangles as above.
    const auto &mesh = bent->GetMesh();
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
    EXPECT_EQ(48U, mesh.points.size());
    EXPECT_EQ(92U, mesh.GetTriangleCount());

    const Bounds bounds = bent->GetScaledBounds();
    EXPECT_EQ(Point3f::Zero(), bounds.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(19.2488f, 4, 10.4992f), bounds.GetSize());

    // Translation should not have changed, but the offset should compensate
    // for the curve.
    EXPECT_EQ(Vector3f(0, 1, 0), bent->GetTranslation());
    EXPECT_VECS_CLOSE(Vector3f(-1.01344f, 0, 3.37566f),
                      bent->GetObjectCenterOffset());
}

TEST_F(BentModelTest, Bend90Max) {
    // 10x2x2 box at (0,1,0).
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(10, 2, 2));
    box->SetTranslation(Vector3f(0, 1, 0));

    // Bend 90 degrees counterclockwise around +Y with the center at the right
    // end.  This should create a 90-degree curve ending near Z=5.
    Spin spin;
    spin.center.Set(5, 0, 0);
    spin.angle = Anglef::FromDegrees(90);

    BentModelPtr bent = Model::CreateModel<BentModel>();
    bent->SetComplexity(.09f);
    bent->SetSpin(spin);
    bent->SetOperandModel(box);

    EXPECT_EQ(spin, bent->GetSpin());

    // Should be the same number of points and triangles as above.
    const auto &mesh = bent->GetMesh();
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
    EXPECT_EQ(48U, mesh.points.size());
    EXPECT_EQ(92U, mesh.GetTriangleCount());

    const Bounds bounds = bent->GetScaledBounds();
    EXPECT_EQ(Point3f::Zero(), bounds.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(19.2488f, 4, 10.4992f), bounds.GetSize());


    // Translation should not have changed, but the offset should compensate
    // for the curve.
    EXPECT_EQ(Vector3f(0, 1, 0), bent->GetTranslation());

    EXPECT_VECS_CLOSE(Vector3f(1.01344f, 0, -3.37567f),
                      bent->GetObjectCenterOffset());
}

TEST_F(BentModelTest, Bend360) {
    // 10x2x2 box at (0,1,0).
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(10, 2, 2));
    box->SetTranslation(Vector3f(0, 1, 0));

    // Bend 360 degrees counterclockwise around +Y with the center at the left
    // end.  This should create a ring.
    Spin spin;
    spin.center.Set(-5, 0, 0);
    spin.angle = Anglef::FromDegrees(360);

    BentModelPtr bent = Model::CreateModel<BentModel>();
    bent->SetComplexity(.09f);
    bent->SetSpin(spin);
    bent->SetOperandModel(box);

    EXPECT_EQ(spin, bent->GetSpin());

    // Should be similar number of points and triangles as above, but coplanar
    // end triangles removed.
    {
        const auto &mesh = bent->GetMesh();
        EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
        EXPECT_EQ(44U, mesh.points.size());
        EXPECT_EQ(88U, mesh.GetTriangleCount());

        const Bounds bounds = bent->GetScaledBounds();
        EXPECT_EQ(Point3f::Zero(), bounds.GetCenter());
        EXPECT_VECS_CLOSE(Vector3f(10.3662f, 4, 8.97738f), bounds.GetSize());
    }

    // Repeat with -360.
    spin.angle = Anglef::FromDegrees(-360);
    bent->SetSpin(spin);
    {
        const auto &mesh = bent->GetMesh();
        EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
        EXPECT_EQ(44U, mesh.points.size());
        EXPECT_EQ(88U, mesh.GetTriangleCount());

        const Bounds bounds = bent->GetScaledBounds();
        EXPECT_EQ(Point3f::Zero(), bounds.GetCenter());
        EXPECT_VECS_CLOSE(Vector3f(10.3662f, 4, 8.97738f), bounds.GetSize());
    }
}

TEST_F(BentModelTest, BendCyl360) {
    // 2x10x2 cylinder at (0,5,0).
    ModelPtr cyl = Model::CreateModel<CylinderModel>();
    cyl->SetComplexity(.01f);  // 4 sides.
    cyl->SetScale(Vector3f(2, 10, 2));
    cyl->SetTranslation(Vector3f(0, 5, 0));

    // Bend 360 degrees counterclockwise around +X with the center in the
    // middle.  This should create a ring.
    Spin spin;
    spin.axis  = Vector3f::AxisX();
    spin.angle = Anglef::FromDegrees(360);

    BentModelPtr bent = Model::CreateModel<BentModel>();
    bent->SetComplexity(.09f);
    bent->SetSpin(spin);
    bent->SetOperandModel(cyl);

    EXPECT_EQ(spin, bent->GetSpin());

    // Should be similar number of points and triangles as above, but coplanar
    // end triangles removed.
    {
        const auto &mesh = bent->GetMesh();
        EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
        EXPECT_EQ(44U, mesh.points.size());
        EXPECT_EQ(88U, mesh.GetTriangleCount());

        const Bounds bounds = bent->GetScaledBounds();
        EXPECT_EQ(Point3f::Zero(), bounds.GetCenter());
        EXPECT_VECS_CLOSE(Vector3f(4, 8.97738f, 10.3662f), bounds.GetSize());
    }

    // Repeat with -360.
    spin.angle = Anglef::FromDegrees(-360);
    bent->SetSpin(spin);
    {
        const auto &mesh = bent->GetMesh();
        EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
        EXPECT_EQ(44U, mesh.points.size());
        EXPECT_EQ(88U, mesh.GetTriangleCount());

        const Bounds bounds = bent->GetScaledBounds();
        EXPECT_EQ(Point3f::Zero(), bounds.GetCenter());
        EXPECT_VECS_CLOSE(Vector3f(4, 8.97738f, 10.3662f), bounds.GetSize());
    }
}

TEST_F(BentModelTest, Bend180OffCenter) {
    // This was resulting in an inside-out mesh.

    // 10x1x1 box at (0,1,0).
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(10, 1, 1));
    box->SetTranslation(Vector3f(0, 1, 0));

    // Bend 180 degrees counterclockwise around +Y with the center at Z=10.
    Spin spin;
    spin.axis  = Vector3f::AxisY();
    spin.angle = Anglef::FromDegrees(180);
    spin.center.Set(0, 0, 10);

    BentModelPtr bent = Model::CreateModel<BentModel>();
    bent->SetComplexity(.09f);
    bent->SetSpin(spin);
    bent->SetOperandModel(box);

    EXPECT_EQ(spin, bent->GetSpin());

    {
        const auto &mesh = bent->GetMesh();
        EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));

        // Make sure the mesh is not inside-out.
        const auto &p0 = mesh.points[mesh.indices[0]];
        const auto &p1 = mesh.points[mesh.indices[1]];
        const auto &p2 = mesh.points[mesh.indices[2]];
        EXPECT_VECS_CLOSE(Vector3f(0, 0, 1), ComputeNormal(p0, p1, p2));
    }

    // Bend 180 degrees clockwise.
    spin.angle = Anglef::FromDegrees(-180);
    bent->SetSpin(spin);
    EXPECT_EQ(spin, bent->GetSpin());

    {
        const auto &mesh = bent->GetMesh();
        EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));

        // Make sure the mesh is not inside-out.
        const auto &p0 = mesh.points[mesh.indices[0]];
        const auto &p1 = mesh.points[mesh.indices[1]];
        const auto &p2 = mesh.points[mesh.indices[2]];
        EXPECT_VECS_CLOSE(Vector3f(0, 0, 1), ComputeNormal(p0, p1, p2));
    }
}
