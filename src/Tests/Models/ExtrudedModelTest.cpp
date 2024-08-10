#include "Math/Profile.h"
#include "Models/ExtrudedModel.h"
#include "Tests/Testing.h"
#include "Tests/SceneTestBase.h"

/// \ingroup Tests
class ExtrudedModelTest : public SceneTestBase {};

TEST_F(ExtrudedModelTest, Defaults) {
    ExtrudedModelPtr extruded = Model::CreateModel<ExtrudedModel>();
    EXPECT_FALSE(extruded->CanSetComplexity());

    // Should be using a default profile.
    EXPECT_TRUE(extruded->GetProfile().IsValid());
}

TEST_F(ExtrudedModelTest, SetProfile) {
    ExtrudedModelPtr extruded = Model::CreateModel<ExtrudedModel>();

    // Create a triangle Profile.
    extruded->SetProfile(ExtrudedModel::CreateProfile(Profile::PointVec{
                Point2f(.2f, .2f), Point2f(.8f, .2f), Point2f(.2f, .8f) }));
    EXPECT_TRUE(extruded->GetProfile().IsValid());
    auto mesh = extruded->GetMesh();
    EXPECT_EQ(6U, mesh.points.size());
    EXPECT_EQ(8U, mesh.GetTriangleCount());

    // Try a regular polygon Profile (triangle).
    extruded->SetProfile(ExtrudedModel::CreateRegularPolygonProfile(3));
    EXPECT_TRUE(extruded->GetProfile().IsValid());
    mesh = extruded->GetMesh();
    EXPECT_EQ(6U,  mesh.points.size());
    EXPECT_EQ(8U, mesh.GetTriangleCount());

    // Try a regular polygon Profile (square).
    extruded->SetProfile(ExtrudedModel::CreateRegularPolygonProfile(4));
    EXPECT_TRUE(extruded->GetProfile().IsValid());
    mesh = extruded->GetMesh();
    EXPECT_EQ(8U,  mesh.points.size());
    EXPECT_EQ(12U, mesh.GetTriangleCount());
}

TEST_F(ExtrudedModelTest, IsValid) {
    SetParseTypeName("ExtrudedModel");
    TestInvalid("profile_points: [2 2]", "Invalid profile");
    TestValid("profile_points: [.2 .2, .8 .2, .2 .8]");
}
