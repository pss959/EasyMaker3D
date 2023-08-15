#include "Models/CylinderModel.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class CylinderModelTest : public SceneTestBase {};

TEST_F(CylinderModelTest, Defaults) {
    CylinderModelPtr cm = Model::CreateModel<CylinderModel>();
    EXPECT_TRUE(cm->CanSetComplexity());
    EXPECT_EQ(1, cm->GetTopRadius());
    EXPECT_EQ(1, cm->GetBottomRadius());

    const TriMesh mesh = cm->GetMesh();
    EXPECT_EQ(78U,  mesh.points.size());
    EXPECT_EQ(152U, mesh.GetTriangleCount());
    EXPECT_EQ(Vector3f::Zero(), cm->GetObjectCenterOffset());

    const auto bounds = cm->GetBounds();
    EXPECT_EQ(Vector3f(2, 2, 2), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
}

TEST_F(CylinderModelTest, SetRadii) {
    CylinderModelPtr cm = Model::CreateModel<CylinderModel>();
    cm->SetTopRadius(.5f);
    cm->SetBottomRadius(2);
    EXPECT_EQ(.5f, cm->GetTopRadius());
    EXPECT_EQ(2,   cm->GetBottomRadius());

    const TriMesh mesh = cm->GetMesh();
    EXPECT_EQ(78U,  mesh.points.size());
    EXPECT_EQ(152U, mesh.GetTriangleCount());
    EXPECT_EQ(Vector3f::Zero(), cm->GetObjectCenterOffset());

    const auto bounds = cm->GetBounds();
    EXPECT_EQ(Vector3f(4, 2, 4), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
}

TEST_F(CylinderModelTest, IsValid) {
    SetParseTypeName("CylinderModel");
    TestInvalid("top_radius: -1", "Negative radius");
    TestInvalid("top_radius: 0, bottom_radius: 0",
                "At least one radius must be positive");
    TestValid("top_radius: 0, bottom_radius: 2");
}
