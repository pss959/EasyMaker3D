#include "Testing.h"
#include "Models/BoxModel.h"
#include "Models/ClippedModel.h"
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
