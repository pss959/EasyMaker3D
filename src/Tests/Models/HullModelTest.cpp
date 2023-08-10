#include "Models/BoxModel.h"
#include "Models/HullModel.h"
#include "Tests/Testing.h"
#include "Tests/SceneTestBase.h"

/// \ingroup Tests
class HullModelTest : public SceneTestBase {};

TEST_F(HullModelTest, TwoBoxes) {
    ModelPtr box0 = Model::CreateModel<BoxModel>();
    ModelPtr box1 = Model::CreateModel<BoxModel>();
    box0->SetUniformScale(4);
    box1->SetUniformScale(4);
    box1->SetTranslation(Vector3f(10, 0, 0));

    auto hull = Model::CreateModel<HullModel>();
    EXPECT_EQ(1U, hull->GetMinChildCount());
    hull->SetOperandModels(std::vector<ModelPtr>{ box0, box1 });
    EXPECT_EQ(2U,   hull->GetOperandModels().size());
    EXPECT_EQ(box0, hull->GetOperandModels()[0]);
    EXPECT_EQ(box1, hull->GetOperandModels()[1]);

    // Validate the resulting mesh.
    auto mesh = hull->GetMesh();
    EXPECT_EQ(8U, mesh.points.size());
    EXPECT_EQ(12U, mesh.GetTriangleCount());

    // And its bounds.
    auto bounds = hull->GetBounds();
    EXPECT_EQ(Point3f(0, 0, 0),   bounds.GetCenter());
    EXPECT_EQ(Vector3f(18, 8, 8), bounds.GetSize());
    EXPECT_EQ(Vector3f(5, 0, 0),  hull->GetObjectCenterOffset());
}

TEST_F(HullModelTest, AddRemove) {
    ModelPtr box0 = Model::CreateModel<BoxModel>();
    ModelPtr box1 = Model::CreateModel<BoxModel>();
    box0->SetUniformScale(4);
    box1->SetUniformScale(4);
    box1->SetTranslation(Vector3f(10, 0, 0));

    // Start with just box0.
    auto hull = Model::CreateModel<HullModel>();
    EXPECT_EQ(0U, hull->GetOperandModels().size());
    hull->AddChildModel(box0);
    EXPECT_EQ(1U,   hull->GetOperandModels().size());
    EXPECT_EQ(box0, hull->GetOperandModels()[0]);
    auto mesh = hull->GetMesh();
    EXPECT_EQ(8U, mesh.points.size());
    EXPECT_EQ(12U, mesh.GetTriangleCount());
    auto bounds = hull->GetBounds();
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
    EXPECT_EQ(Vector3f(8, 8, 8), bounds.GetSize());
    EXPECT_EQ(Vector3f(0, 0, 0), hull->GetObjectCenterOffset());

    // Add box1 with InsertChildModel().
    hull->InsertChildModel(1, box1);
    EXPECT_EQ(2U,   hull->GetOperandModels().size());
    EXPECT_EQ(box0, hull->GetOperandModels()[0]);
    EXPECT_EQ(box1, hull->GetOperandModels()[1]);
    mesh = hull->GetMesh();
    EXPECT_EQ(8U, mesh.points.size());
    EXPECT_EQ(12U, mesh.GetTriangleCount());
    bounds = hull->GetBounds();
    EXPECT_EQ(Point3f(0, 0, 0),   bounds.GetCenter());
    EXPECT_EQ(Vector3f(18, 8, 8), bounds.GetSize());
    EXPECT_EQ(Vector3f(5, 0, 0),  hull->GetObjectCenterOffset());

    // Remove box0.
    hull->RemoveChildModel(0);
    EXPECT_EQ(1U,   hull->GetOperandModels().size());
    EXPECT_EQ(box1, hull->GetOperandModels()[0]);
    mesh = hull->GetMesh();
    EXPECT_EQ(8U, mesh.points.size());
    EXPECT_EQ(12U, mesh.GetTriangleCount());
    bounds = hull->GetBounds();
    EXPECT_EQ(Point3f(0, 0, 0),   bounds.GetCenter());
    EXPECT_EQ(Vector3f(8, 8, 8),  bounds.GetSize());
    EXPECT_EQ(Vector3f(10, 0, 0), hull->GetObjectCenterOffset());

    // Replace box1 with box0.
    hull->ReplaceChildModel(0, box0);
    EXPECT_EQ(1U,   hull->GetOperandModels().size());
    EXPECT_EQ(box0, hull->GetOperandModels()[0]);
    mesh = hull->GetMesh();
    EXPECT_EQ(8U, mesh.points.size());
    EXPECT_EQ(12U, mesh.GetTriangleCount());
    bounds = hull->GetBounds();
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
    EXPECT_EQ(Vector3f(8, 8, 8), bounds.GetSize());
    EXPECT_EQ(Vector3f(0, 0, 0), hull->GetObjectCenterOffset());

    // Insert box1 back in.
    hull->InsertChildModel(0, box1);
    EXPECT_EQ(2U,   hull->GetOperandModels().size());
    EXPECT_EQ(box1, hull->GetOperandModels()[0]);
    EXPECT_EQ(box0, hull->GetOperandModels()[1]);
    mesh = hull->GetMesh();
    EXPECT_EQ(8U, mesh.points.size());
    EXPECT_EQ(12U, mesh.GetTriangleCount());
    bounds = hull->GetBounds();
    EXPECT_EQ(Point3f(0, 0, 0),   bounds.GetCenter());
    EXPECT_EQ(Vector3f(18, 8, 8), bounds.GetSize());
    EXPECT_EQ(Vector3f(5, 0, 0),  hull->GetObjectCenterOffset());
}

TEST_F(HullModelTest, IsValid) {
    TEST_THROW(ParseObject<HullModel>("HullModel {}"),
               Parser::Exception, "Only 0 operand model(s)");

    // This should not throw.
    auto hull = ParseObject<HullModel>(
        "HullModel { operand_models: [ BoxModel {} ] }");
    EXPECT_NOT_NULL(hull);
}
