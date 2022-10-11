#include "Testing.h"
#include "Models/BoxModel.h"
#include "Models/ClippedModel.h"
#include "Models/CylinderModel.h"
#include "SceneTestBase.h"

class ClippedModelTest : public SceneTestBase {
};

TEST_F(ClippedModelTest, NoPlanes) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetUniformScale(4);
    box->SetTranslation(Vector3f(0, 4, 0));
    Bounds bounds = box->GetBounds();
    EXPECT_EQ(Vector3f(2, 2, 2), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());

    ClippedModelPtr clipped = Model::CreateModel<ClippedModel>();
    clipped->SetOriginalModel(box);
    clipped->SetStatus(Model::Status::kUnselected);

    bounds = clipped->GetBounds();
    EXPECT_EQ(Vector3f(8, 8, 8), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
    EXPECT_EQ(Vector3f(1, 1, 1), clipped->GetScale());
    EXPECT_EQ(Vector3f(0, 4, 0), clipped->GetTranslation());
}

TEST_F(ClippedModelTest, OnePlane) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetUniformScale(4);
    box->SetTranslation(Vector3f(0, 4, 0));

    ClippedModelPtr clipped = Model::CreateModel<ClippedModel>();
    clipped->SetOriginalModel(box);

    // This should clip away the top half.
    clipped->AddPlane(Plane(4, Vector3f::AxisY()));
    clipped->SetStatus(Model::Status::kUnselected);

    Bounds bounds = clipped->GetBounds();
    EXPECT_EQ(Vector3f(8, 4, 8), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
    EXPECT_EQ(Vector3f(1, 1, 1), clipped->GetScale());
    EXPECT_EQ(Vector3f(0, 2, 0), clipped->GetTranslation());
}

TEST_F(ClippedModelTest, TwoPlanes) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetUniformScale(4);
    box->SetTranslation(Vector3f(0, 4, 0));

    ClippedModelPtr clipped = Model::CreateModel<ClippedModel>();
    clipped->SetOriginalModel(box);
    clipped->AddPlane(Plane(4, Vector3f::AxisY()));
    clipped->AddPlane(Plane(-2, Vector3f::AxisX()));
    clipped->SetStatus(Model::Status::kUnselected);

    Bounds bounds = clipped->GetBounds();
    EXPECT_EQ(Vector3f(2, 4, 8),  bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),   bounds.GetCenter());
    EXPECT_EQ(Vector3f(1, 1, 1),  clipped->GetScale());
    EXPECT_EQ(Vector3f(-3, 2, 0), clipped->GetTranslation());
}

TEST_F(ClippedModelTest, Cylinder) {
    // This was causing CGAL self-intersections before adding code to
    // TransformPlane to clean up the normal and distance.
    ModelPtr cyl = Model::CreateModel<CylinderModel>();
    cyl->SetUniformScale(2);
    cyl->SetTranslation(Vector3f(10, 0, 0));
    cyl->SetComplexity(0.0864977f);

    ClippedModelPtr clipped = Model::CreateModel<ClippedModel>();
    clipped->SetOriginalModel(cyl);
    clipped->AddPlane(Plane(1.33878e-09f,
                            Vector3f(5.96047e-08f, 1, 1.19209e-07f)));
    clipped->SetStatus(Model::Status::kUnselected);

    std::string reason;
    EXPECT_TRUE(clipped->IsMeshValid(reason));
}
