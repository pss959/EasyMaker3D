#include "Testing.h"
#include "Models/BoxModel.h"
#include "Models/BeveledModel.h"
#include "SceneTestBase.h"

class BeveledModelTest : public SceneTestBase {
};

TEST_F(BeveledModelTest, DefaultBevel) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetUniformScale(4);
    box->SetTranslation(Vector3f(0, 4, 0));
    Bounds bounds = box->GetBounds();
    EXPECT_EQ(Vector3f(2, 2, 2), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());

    BeveledModelPtr beveled = Model::CreateModel<BeveledModel>();
    beveled->SetOriginalModel(box);

    // Make sure the BeveledModel is considered visible.
    beveled->SetStatus(Model::Status::kUnselected);

    // See BevelerTest for explanation of counts.
    TriMesh mesh = beveled->GetMesh();
    EXPECT_EQ(24U, mesh.points.size());
    EXPECT_EQ(44U, mesh.GetTriangleCount());

    bounds = beveled->GetBounds();
    EXPECT_EQ(Vector3f(8, 8, 8), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
    EXPECT_EQ(Vector3f(1, 1, 1), beveled->GetScale());
    EXPECT_EQ(Vector3f(0, 4, 0), beveled->GetTranslation());
}
