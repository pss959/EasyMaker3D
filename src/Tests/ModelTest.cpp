#include "Defaults.h"
#include "Testing.h"
#include "Models/BoxModel.h"
#include "Models/CylinderModel.h"
#include "SceneTestBase.h"

class ModelTest : public SceneTestBase {
};

TEST_F(ModelTest, Use) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    EXPECT_EQ(Model::Use::kNew, box->GetUse());

    for (auto use: Util::EnumValues<Model::Use>()) {
        box->SetUse(use);
        EXPECT_EQ(use, box->GetUse());
    }
}

TEST_F(ModelTest, Status) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    EXPECT_EQ(Model::Status::kUnknown, box->GetStatus());

    for (auto status: Util::EnumValues<Model::Status>()) {
        box->SetStatus(status);
        EXPECT_EQ(status, box->GetStatus());
    }
}

TEST_F(ModelTest, Level) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    EXPECT_EQ(0, box->GetLevel());
    EXPECT_FALSE(box->IsTopLevel());
    box->SetLevel(1);
    EXPECT_EQ(1, box->GetLevel());
    EXPECT_TRUE(box->IsTopLevel());
    box->SetLevel(2);
    EXPECT_EQ(2, box->GetLevel());
    EXPECT_FALSE(box->IsTopLevel());
}

TEST_F(ModelTest, Complexity) {
    // BoxModel does not respond to complexity, but CylinderModel does.
    ModelPtr box = Model::CreateModel<BoxModel>();
    ModelPtr cyl = Model::CreateModel<CylinderModel>();
    EXPECT_FALSE(box->CanSetComplexity());
    EXPECT_TRUE(cyl->CanSetComplexity());

    EXPECT_EQ(Defaults::kModelComplexity, box->GetComplexity());
    EXPECT_EQ(Defaults::kModelComplexity, cyl->GetComplexity());

    box->SetComplexity(.2f);
    cyl->SetComplexity(.2f);
    EXPECT_EQ(Defaults::kModelComplexity, box->GetComplexity());
    EXPECT_EQ(.2f,                        cyl->GetComplexity());
}

TEST_F(ModelTest, Bounds) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    const Bounds bounds = box->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(Point3f::Zero(),   bounds.GetCenter());
    EXPECT_EQ(Vector3f(2, 2, 2), bounds.GetSize());
}

TEST_F(ModelTest, MoveTo) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    Vector3f   scale(1, 2, 3);
    Rotationf  rot = Rotationf::FromEulerAngles(Anglef::FromDegrees(20),
                                                Anglef::FromDegrees(30),
                                                Anglef::FromDegrees(-40));
    Vector3f   trans(100, 200, 300);
    box->SetScale(scale);
    box->SetRotation(rot);
    box->SetTranslation(trans);

    // Move the center of the model. Rotation and scale should be untouched.
    Point3f c(-10, 3, 14);
    box->MoveCenterTo(c);
    EXPECT_EQ(scale, box->GetScale());
    EXPECT_EQ(rot,   box->GetRotation());
    EXPECT_EQ(c,     Point3f(box->GetTranslation()));

    // Move the bottom center. This should also change rotation.
    box->MoveBottomCenterTo(c, Vector3f(0, 1, 0));
    EXPECT_EQ(scale, box->GetScale());
    EXPECT_TRUE(box->GetRotation().IsIdentity());
    EXPECT_EQ(c + Vector3f(0, scale[1], 0), Point3f(box->GetTranslation()));
}

#if XXXX
    [Test]
    public void Bounds() {
        Model box = modelMgr.CreatePrimitiveModel(PrimitiveType.Box);
        box.SetLocalScale(4f * Vector3.one);
        box.SetLocalPosition(10f * Vector3.right);
        CompareBounds(new Bounds(Vector3.zero, 2f * Vector3.one),
                      box.GetObjectBounds());
        CompareBounds(new Bounds(Vector3.zero, 8f * Vector3.one),
                      box.GetScaledObjectBounds());
        CompareBounds(new Bounds(10f * Vector3.right, 8f * Vector3.one),
                      box.GetStageBounds());
        CompareVector3(10f * Vector3.right, box.GetStageCenter());
    }

    [Test]

    [Test]
    public void Click() {
        string n = "";
        Model m = Model::CreateModel();
        box->name = "TestName";
        box->Clicked.AddListener((w, info) => { n = (w as Model).name; });
        box->Click(new ClickInfo());
        Assert.AreEqual("TestName", n);
    }
}
#endif
