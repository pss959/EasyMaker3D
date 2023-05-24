#include "Math/Bend.h"
#include "Models/BoxModel.h"
#include "Models/BentModel.h"
#include "Tests/Testing.h"
#include "Tests/SceneTestBase.h"
#include "Util/General.h"
#include "Util/Tuning.h"

class BentModelTest : public SceneTestBase {
};

TEST_F(BentModelTest, DefaultBend) {
    // 2x10x2 box at (0,5,0).
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(2, 10, 2));
    box->SetTranslation(Vector3f(0, 5, 0));

    BentModelPtr bent = Model::CreateModel<BentModel>();
    bent->SetOperandModel(box);

    // Should be using a default bend.
    Bend default_bend;
    EXPECT_EQ(default_bend, bent->GetBend());

    // The result should be the original box, but sliced.
    const auto &mesh = bent->GetMesh();
    EXPECT_EQ(48U, mesh.points.size());
    EXPECT_EQ(92U, mesh.GetTriangleCount());

    // Bounds should not have changed.
    const Bounds bounds = bent->GetScaledBounds();
    EXPECT_EQ(Point3f::Zero(),    bounds.GetCenter());
    EXPECT_EQ(Vector3f(4, 20, 4), bounds.GetSize());
}

TEST_F(BentModelTest, Bend90) {
    // 2x10x2 box at (0,5,0).
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(2, 10, 2));
    box->SetTranslation(Vector3f(0, 5, 0));

    Bend bend;
    bend.center.Set(4, 0, 0);
    bend.angle = Anglef::FromDegrees(-90);

    BentModelPtr bent = Model::CreateModel<BentModel>();
    bent->SetBend(bend);
    bent->SetOperandModel(box);

    bent->SetComplexity(0); // XXXX TEMPORARY!

    EXPECT_EQ(bend, bent->GetBend());

    // Should be the same number of points and triangles as above.
    const auto &mesh = bent->GetMesh();
    EXPECT_EQ(48U, mesh.points.size());
    EXPECT_EQ(92U, mesh.GetTriangleCount());

    // The top 4 points of the box should be at the right


    // Bounds should be XXXX
    const Bounds bounds = bent->GetScaledBounds();
    EXPECT_EQ(Point3f::Zero(),    bounds.GetCenter());
    EXPECT_EQ(Vector3f(10, 10, 4), bounds.GetSize());
}
