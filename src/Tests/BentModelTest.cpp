#include "Math/Bend.h"
#include "Math/MeshValidation.h"
#include "Models/BoxModel.h"
#include "Models/BentModel.h"
#include "Tests/Testing.h"
#include "Tests/SceneTestBase.h"
#include "Util/General.h"
#include "Util/Tuning.h"

#include "Debug/Dump3dv.h" // XXXX

class BentModelTest : public SceneTestBase {
};

TEST_F(BentModelTest, DefaultBend) {
    // 10x2x2 box at (0,1,0).
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(10, 2, 2));
    box->SetTranslation(Vector3f(0, 1, 0));

    BentModelPtr bent = Model::CreateModel<BentModel>();
    bent->SetOperandModel(box);

    // Should be using a default bend.
    Bend default_bend;
    EXPECT_EQ(default_bend, bent->GetBend());

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

TEST_F(BentModelTest, Bend90) {
    // 10x2x2 box at (0,1,0).
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(10, 2, 2));
    box->SetTranslation(Vector3f(0, 1, 0));

    // Bend 90 degrees clockwise around +Y with the center at the left end.
    // This should create a 90-degree curve ending near Z=5.
    Bend bend;
    bend.center.Set(-5, 0, 0);
    bend.angle = Anglef::FromDegrees(-90);

    BentModelPtr bent = Model::CreateModel<BentModel>();
    bent->SetBend(bend);
    bent->SetOperandModel(box);

    EXPECT_EQ(bend, bent->GetBend());

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
    Bend bend;
    bend.center.Set(5, 0, 0);
    bend.angle = Anglef::FromDegrees(90);

    BentModelPtr bent = Model::CreateModel<BentModel>();
    bent->SetBend(bend);
    bent->SetOperandModel(box);

    EXPECT_EQ(bend, bent->GetBend());

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
    EXPECT_VECS_CLOSE(Vector3f(1.01344f, 0, -3.37566f),
                      bent->GetObjectCenterOffset());

#if 1 // XXXX
        {
            Debug::Dump3dv d("/tmp/bent.3dv", "XXXX From BentModelTest");
            d.SetLabelFontSize(30);
            d.SetCoincidentLabelOffset(.25f * Vector3f(1, 1, 1));
            d.AddTriMesh(mesh);
        }
#endif
}

TEST_F(BentModelTest, Bend360) {
    // 10x2x2 box at (0,1,0).
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(10, 2, 2));
    box->SetTranslation(Vector3f(0, 1, 0));

    // Bend 360 degrees counterclockwise around +Y with the center at the left
    // end.  This should create a ring.
    Bend bend;
    bend.center.Set(-5, 0, 0);
    bend.angle = Anglef::FromDegrees(360);

    BentModelPtr bent = Model::CreateModel<BentModel>();
    bent->SetBend(bend);
    bent->SetOperandModel(box);

    EXPECT_EQ(bend, bent->GetBend());

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
    bend.angle = Anglef::FromDegrees(-360);
    bent->SetBend(bend);
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
