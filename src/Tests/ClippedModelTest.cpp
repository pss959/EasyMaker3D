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

    ClippedModelPtr clipped = Model::CreateModel<ClippedModel>();
    clipped->SetOriginalModel(box);

    Bounds bounds = clipped->GetBounds();
    EXPECT_EQ(Vector3f(8, 8, 8), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 4, 0),  bounds.GetCenter());
}

TEST_F(ClippedModelTest, OnePlane) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetUniformScale(4);
    box->SetTranslation(Vector3f(0, 4, 0));

    ClippedModelPtr clipped = Model::CreateModel<ClippedModel>();
    clipped->SetOriginalModel(box);
    clipped->AddPlane(Plane(0, Vector3f::AxisY()));

    Bounds bounds = clipped->GetBounds();
    EXPECT_EQ(Vector3f(8, 4, 8), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 2, 0),  bounds.GetCenter());
}
