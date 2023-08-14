#include "Models/BoxModel.h"
#include "Models/CSGModel.h"
#include "Tests/Testing.h"
#include "Tests/SceneTestBase.h"

/// \ingroup Tests
class CSGModelTest : public SceneTestBase {};

TEST_F(CSGModelTest, TwoBoxes) {
    ModelPtr box0 = Model::CreateModel<BoxModel>();
    ModelPtr box1 = Model::CreateModel<BoxModel>();
    box0->SetUniformScale(4);
    box1->SetUniformScale(4);
    box1->SetTranslation(Vector3f(1, 2, 3));

    auto csg = Model::CreateModel<CSGModel>();
    EXPECT_EQ(2U, csg->GetMinChildCount());
    EXPECT_EQ(CSGOperation::kUnion, csg->GetOperation());
    csg->SetOperandModels(std::vector<ModelPtr>{ box0, box1 });

    // Validate the resulting mesh.
    auto mesh = csg->GetMesh();
    EXPECT_EQ(29U, mesh.points.size());
    EXPECT_EQ(54U, mesh.GetTriangleCount());

    // And its bounds.
    auto bounds = csg->GetBounds();
    EXPECT_EQ(Point3f(0, 0, 0),       bounds.GetCenter());
    EXPECT_EQ(Vector3f(9, 10, 11),    bounds.GetSize());
    EXPECT_EQ(Vector3f(.5f, 1, 1.5f), csg->GetObjectCenterOffset());

    // Switch to difference.
    csg->SetOperation(CSGOperation::kDifference);
    EXPECT_EQ(CSGOperation::kDifference, csg->GetOperation());
    mesh = csg->GetMesh();
    EXPECT_EQ(23U, mesh.points.size());
    EXPECT_EQ(42U, mesh.GetTriangleCount());
    bounds = csg->GetBounds();
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
    EXPECT_EQ(Vector3f(8, 8, 8), bounds.GetSize());
    EXPECT_EQ(Vector3f(0, 0, 0), csg->GetObjectCenterOffset());

    // Switch to intersection.
    csg->SetOperation(CSGOperation::kIntersection);
    EXPECT_EQ(CSGOperation::kIntersection, csg->GetOperation());
    mesh = csg->GetMesh();
    EXPECT_EQ(17U, mesh.points.size());
    EXPECT_EQ(30U, mesh.GetTriangleCount());
    bounds = csg->GetBounds();
    EXPECT_EQ(Point3f(0, 0, 0),       bounds.GetCenter());
    EXPECT_EQ(Vector3f(7, 6, 5),      bounds.GetSize());
    EXPECT_EQ(Vector3f(.5f, 1, 1.5f), csg->GetObjectCenterOffset());
}

TEST_F(CSGModelTest, IsValid) {
    TestInvalid("CSGModel {}", "Only 0 operand model(s)");
    TestInvalid("CSGModel { operand_models: [ BoxModel {} ]}",
                "Only 1 operand model(s)");
    TestValid("CSGModel { operand_models: [ BoxModel {}, BoxModel {} ] }");
}
