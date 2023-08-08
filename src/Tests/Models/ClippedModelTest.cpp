#include "Tests/Testing.h"
#include "Models/BoxModel.h"
#include "Models/ClippedModel.h"
#include "Models/CylinderModel.h"
#include "Tests/SceneTestBase.h"

/// \ingroup Tests
class ClippedModelTest : public SceneTestBase {};

TEST_F(ClippedModelTest, DefaultPlane) {
    // 8x8x8 box at (0,4,0).
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetUniformScale(4);
    box->SetTranslation(Vector3f(0, 4, 0));

    ClippedModelPtr clipped = Model::CreateModel<ClippedModel>();
    clipped->SetOperandModel(box);

    // The default plane should clip away the top half.
    EXPECT_EQ(ClippedModel::GetDefaultPlane(), clipped->GetPlane());

    Bounds bounds = clipped->GetBounds();
    EXPECT_EQ(Vector3f(2, 1, 2), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
    EXPECT_EQ(Vector3f(4, 4, 4), clipped->GetScale());
    EXPECT_EQ(Vector3f(0, 2, 0), clipped->GetTranslation());

    // Verify offset vectors.
    EXPECT_EQ(Vector3f(0, -.5f, 0), clipped->GetObjectCenterOffset());
    EXPECT_EQ(Vector3f(0,   -2, 0), clipped->GetLocalCenterOffset());
}

TEST_F(ClippedModelTest, OtherPlane) {
    // 4x4x4 box at (0,4,0).
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetUniformScale(2);
    box->SetTranslation(Vector3f(0, 4, 0));

    ClippedModelPtr clipped = Model::CreateModel<ClippedModel>();

    // Clip the right quarter.
    const Plane plane(.5f, Vector3f::AxisX());
    clipped->SetPlane(plane);

    // Do this last so transforms are synced.
    clipped->SetOperandModel(box);

    EXPECT_EQ(plane, clipped->GetPlane());

    Bounds bounds = clipped->GetBounds();
    EXPECT_EQ(Vector3f(1.5f, 2, 2), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),     bounds.GetCenter());
    EXPECT_EQ(Vector3f(2, 2, 2),    clipped->GetScale());
    EXPECT_EQ(Vector3f(-.5f, 4, 0), clipped->GetTranslation());

    // Verify offset vectors.
    EXPECT_EQ(Vector3f(-.25f, 0, 0), clipped->GetObjectCenterOffset());
    EXPECT_EQ(Vector3f(-.5f,  0, 0), clipped->GetLocalCenterOffset());
}

TEST_F(ClippedModelTest, Cylinder) {
    // This was causing CGAL self-intersections before adding code to
    // TransformPlane to clean up the normal and distance.
    ModelPtr cyl = Model::CreateModel<CylinderModel>();
    cyl->SetUniformScale(2);
    cyl->SetTranslation(Vector3f(10, 0, 0));
    cyl->SetComplexity(0.0864977f);

    ClippedModelPtr clipped = Model::CreateModel<ClippedModel>();
    clipped->SetPlane(Plane(1.33878e-09f,
                            Vector3f(5.96047e-08f, 1, 1.19209e-07f)));
    clipped->SetOperandModel(cyl);

    std::string reason;
    EXPECT_TRUE(clipped->IsMeshValid(reason));
}
