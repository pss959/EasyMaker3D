#include "Models/TorusModel.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/Tuning.h"

/// \ingroup Tests
class TorusModelTest : public SceneTestBase {};

TEST_F(TorusModelTest, Defaults) {
    TorusModelPtr tm = Model::CreateModel<TorusModel>();
    EXPECT_TRUE(tm->CanSetComplexity());
    EXPECT_EQ(TK::kTorusInnerRadius, tm->GetInnerRadius());
    EXPECT_EQ(1,                     tm->GetOuterRadius());

    const TriMesh mesh = tm->GetMesh();
    EXPECT_EQ(1024U, mesh.points.size());
    EXPECT_EQ(2048U, mesh.GetTriangleCount());
    EXPECT_EQ(Vector3f::Zero(), tm->GetObjectCenterOffset());

    const auto bounds = tm->GetBounds();
    EXPECT_VECS_CLOSE(Vector3f(2, .4f, 2), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),            bounds.GetCenter());
}

TEST_F(TorusModelTest, SetRadii) {
    TorusModelPtr tm = Model::CreateModel<TorusModel>();
    tm->SetInnerRadius(.5f);
    tm->SetOuterRadius(2);
    EXPECT_EQ(.5f, tm->GetInnerRadius());
    EXPECT_EQ(2,   tm->GetOuterRadius());

    const TriMesh mesh = tm->GetMesh();
    EXPECT_EQ(1024U, mesh.points.size());
    EXPECT_EQ(2048U, mesh.GetTriangleCount());
    EXPECT_EQ(Vector3f::Zero(), tm->GetObjectCenterOffset());

    const auto bounds = tm->GetBounds();
    EXPECT_EQ(Vector3f(4, 1, 4), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
}

TEST_F(TorusModelTest, IsValid) {
    SetParseTypeName("TorusModel");
    TestInvalid("inner_radius: -1", "Non-positive radius");
    TestInvalid("inner_radius: 1, outer_radius: 1.2",
                "Outer radius too small");
    TestValid("inner_radius: .2, outer_radius: 4");
}
