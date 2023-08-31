#include "Models/BeveledModel.h"
#include "Models/BoxModel.h"
#include "Models/TextModel.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class BeveledModelTest : public SceneTestBase {};

TEST_F(BeveledModelTest, DefaultBevel) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetUniformScale(4);
    box->SetTranslation(Vector3f(0, 4, 0));
    Bounds bounds = box->GetBounds();
    EXPECT_EQ(Vector3f(2, 2, 2), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());

    BeveledModelPtr beveled = Model::CreateModel<BeveledModel>();
    beveled->SetOperandModel(box);
    EXPECT_EQ(Bevel(), beveled->GetBevel());

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

TEST_F(BeveledModelTest, IsValid) {
    SetParseTypeName("BeveledModel");
    TestInvalid("", "No operand model");
    TestInvalid("operand_model: BoxModel {}, bevel_scale: -1",
                "Non-positive scale value");
    TestInvalid("operand_model: BoxModel {}, profile_points: [1 2]",
                "Invalid profile");   // Point out of (0,1) range.
    TestValid("operand_model: BoxModel {}");
}

TEST_F(BeveledModelTest, SetBevel) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetUniformScale(4);
    box->SetTranslation(Vector3f(0, 4, 0));

    BeveledModelPtr beveled = Model::CreateModel<BeveledModel>();
    beveled->SetOperandModel(box);

    // Make sure the BeveledModel is considered visible.
    beveled->SetStatus(Model::Status::kUnselected);

    Bevel bevel;
    bevel.profile.AppendPoint(Point2f(.75f, .75f));
    beveled->SetBevel(bevel);
    EXPECT_EQ(bevel, beveled->GetBevel());

    TriMesh mesh = beveled->GetMesh();
    EXPECT_EQ(48U, mesh.points.size());
    EXPECT_EQ(92U, mesh.GetTriangleCount());
}

TEST_F(BeveledModelTest, ChangeOperandModel) {
    ModelPtr box0 = Model::CreateModel<BoxModel>();
    ModelPtr box1 = Model::CreateModel<BoxModel>();
    box0->SetUniformScale(4);
    box1->SetUniformScale(6);
    box0->SetTranslation(Vector3f(0, 4, 0));
    box1->SetTranslation(Vector3f(0, 6, 0));

    BeveledModelPtr beveled = Model::CreateModel<BeveledModel>();
    beveled->SetOperandModel(box0);
    EXPECT_EQ(box0, beveled->GetOperandModel());
    beveled->SetStatus(Model::Status::kUnselected);

    auto bounds = beveled->GetBounds();
    EXPECT_EQ(Vector3f(8, 8, 8), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
    EXPECT_EQ(Vector3f(1, 1, 1), beveled->GetScale());
    EXPECT_EQ(Vector3f(0, 4, 0), beveled->GetTranslation());

    beveled->SetOperandModel(box1);
    bounds = beveled->GetBounds();
    EXPECT_EQ(box1, beveled->GetOperandModel());
    EXPECT_EQ(Vector3f(12, 12, 12), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
    EXPECT_EQ(Vector3f(1, 1, 1), beveled->GetScale());
    EXPECT_EQ(Vector3f(0, 6, 0), beveled->GetTranslation());
}

TEST_F(BeveledModelTest, ScaleChanges) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    box->SetUniformScale(4);
    box->SetTranslation(Vector3f(0, 4, 0));

    BeveledModelPtr beveled = Model::CreateModel<BeveledModel>();
    beveled->SetOperandModel(box);

    // Make sure the BeveledModel is considered visible.
    beveled->SetStatus(Model::Status::kUnselected);

    // Change the scale in the BeveledModel.
    beveled->SetScale(Vector3f(2, 3, 4));
    EXPECT_EQ(Vector3f(2, 3, 4), beveled->GetScale());
    EXPECT_EQ(Vector3f(4, 4, 4), box->GetScale());
    Bounds bounds = beveled->GetBounds();
    EXPECT_EQ(Vector3f(8, 8, 8), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),    bounds.GetCenter());

    // Change the scale in the operand Box. The BeveledModel should rebuild
    // its mesh.
    box->SetScale(Vector3f(5, 6, 7));
    EXPECT_EQ(Vector3f(2, 3, 4), beveled->GetScale());
    EXPECT_EQ(Vector3f(5, 6, 7), box->GetScale());
    bounds = beveled->GetBounds();
    EXPECT_VECS_CLOSE(Vector3f(10, 12, 14), bounds.GetSize());
    EXPECT_EQ(Point3f(0, 0, 0),     bounds.GetCenter());
}

TEST_F(BeveledModelTest, Text) {
    UseRealFontSystem(true);
    TextModelPtr text = Model::CreateModel<TextModel>();
    text->SetTextString("ABC");

    // Create a BeveledModel using a default Bevel.
    BeveledModelPtr beveled = Model::CreateModel<BeveledModel>();
    beveled->SetOperandModel(text);

    Bounds bounds = beveled->GetBounds();
    EXPECT_EQ(Point3f(0, 0, 0), bounds.GetCenter());
}
