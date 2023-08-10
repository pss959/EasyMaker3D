#include "Math/Profile.h"
#include "Models/BoxModel.h"
#include "Models/RevSurfModel.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class RevSurfModelTest : public SceneTestBase {};

TEST_F(RevSurfModelTest, DefaultProfile) {
    RevSurfModelPtr rsm = Model::CreateModel<RevSurfModel>();
    EXPECT_EQ(RevSurfModel::CreateDefaultProfile(), rsm->GetProfile());
    EXPECT_EQ(360, rsm->GetSweepAngle().Degrees());

    const TriMesh mesh = rsm->GetMesh();
    EXPECT_EQ(40U, mesh.points.size());
    EXPECT_EQ(76U, mesh.GetTriangleCount());
    EXPECT_EQ(Vector3f::Zero(), rsm->GetObjectCenterOffset());

    const auto bounds = rsm->GetBounds();
    EXPECT_EQ(Vector3f(1, 1, 1), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
    EXPECT_EQ(Vector3f(1, 1, 1), rsm->GetScale());
    EXPECT_EQ(Vector3f(0, 0, 0), rsm->GetTranslation());
}

TEST_F(RevSurfModelTest, Profile4Points) {
    const Profile::PointVec pts{
        Point2f(.5f, .75f),
        Point2f(.5f, .25f),
    };
    const Profile p = RevSurfModel::CreateProfile(pts);

    RevSurfModelPtr rsm = Model::CreateModel<RevSurfModel>();
    rsm->SetProfile(p);
    EXPECT_EQ(p, rsm->GetProfile());
    EXPECT_EQ(360, rsm->GetSweepAngle().Degrees());

    const TriMesh mesh = rsm->GetMesh();
    EXPECT_EQ(78U,  mesh.points.size());
    EXPECT_EQ(152U, mesh.GetTriangleCount());
    EXPECT_EQ(Vector3f::Zero(), rsm->GetObjectCenterOffset());

    const auto bounds = rsm->GetBounds();
    EXPECT_EQ(Vector3f(1, 1, 1), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
    EXPECT_EQ(Vector3f(1, 1, 1), rsm->GetScale());
    EXPECT_EQ(Vector3f(0, 0, 0), rsm->GetTranslation());
}

TEST_F(RevSurfModelTest, HalfSweep) {
    const float kExpectedZSize = .4983f;

    RevSurfModelPtr rsm = Model::CreateModel<RevSurfModel>();
    rsm->SetSweepAngle(Anglef::FromDegrees(180));
    EXPECT_EQ(RevSurfModel::CreateDefaultProfile(), rsm->GetProfile());
    EXPECT_EQ(180, rsm->GetSweepAngle().Degrees());

    const TriMesh mesh = rsm->GetMesh();
    EXPECT_EQ(22U, mesh.points.size());
    EXPECT_EQ(40U, mesh.GetTriangleCount());
    // Offset to compensate for partial sweep.
    EXPECT_VECS_CLOSE(Vector3f(0, 0, .5f * kExpectedZSize),
                      rsm->GetObjectCenterOffset());

    const auto bounds = rsm->GetBounds();
    EXPECT_VECS_CLOSE(Vector3f(1, 1, kExpectedZSize), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
    EXPECT_EQ(Vector3f(1, 1, 1), rsm->GetScale());
    EXPECT_EQ(Vector3f(0, 0, 0), rsm->GetTranslation());
}
