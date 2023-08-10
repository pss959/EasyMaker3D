#include "Models/SphereModel.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class SphereModelTest : public SceneTestBase {};

TEST_F(SphereModelTest, Defaults) {
    SphereModelPtr sm = Model::CreateModel<SphereModel>();
    EXPECT_TRUE(sm->CanSetComplexity());

    const TriMesh mesh = sm->GetMesh();
    EXPECT_EQ(382U,  mesh.points.size());
    EXPECT_EQ(760U, mesh.GetTriangleCount());
    EXPECT_EQ(Vector3f::Zero(), sm->GetObjectCenterOffset());

    const auto bounds = sm->GetBounds();
    EXPECT_EQ(Vector3f(2, 2, 2), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
}
