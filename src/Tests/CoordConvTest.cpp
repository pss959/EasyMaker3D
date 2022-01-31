#include <string>

#include "CoordConv.h"
#include "Parser/Registry.h"
#include "RegisterTypes.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "TestBase.h"
#include "Testing.h"

class CoordConvTest : public TestBase {
  public:
    CoordConvTest()  { RegisterTypes(); }
    ~CoordConvTest() { UnregisterTypes(); }

    // Creates and returns a named Node to add to a NodePath.
    SG::NodePtr CreateNode(const std::string &name) {
        return CreateObject<SG::Node>(name);
    }

  protected:
    // Vectors and points used for testing.
    static constexpr Vector3f v0{0, 0, 0};
    static constexpr Vector3f v1{1, 1, 1};
    static constexpr Point3f  p0{0, 0, 0};
    static constexpr Point3f  p1{1, 1, 1};
};

TEST_F(CoordConvTest, ObjectWorld) {
    SG::NodePtr a = CreateNode("A");
    SG::NodePtr b = CreateNode("B");
    a->SetScale(Vector3f(2, 3, 4));
    b->SetScale(Vector3f(10, 100, 1000));
    a->SetTranslation(Vector3f(6, 7, 8));
    b->SetTranslation(Vector3f(21, 22, 23));

    CoordConv cc;

    // One-level path.
    SG::NodePath path;
    path.push_back(a);

    EXPECT_VECS_CLOSE(Vector3f(0, 0, 0), cc.ObjectToWorld(path, v0));
    EXPECT_VECS_CLOSE(Vector3f(2, 3, 4), cc.ObjectToWorld(path, v1));

    EXPECT_PTS_CLOSE(Point3f(6,  7,  8), cc.ObjectToWorld(path, p0));
    EXPECT_PTS_CLOSE(Point3f(8, 10, 12), cc.ObjectToWorld(path, p1));

    EXPECT_VECS_CLOSE(v0, cc.WorldToObject(path, Vector3f(0, 0, 0)));
    EXPECT_VECS_CLOSE(v1, cc.WorldToObject(path, Vector3f(2, 3, 4)));

    EXPECT_PTS_CLOSE(p0, cc.WorldToObject(path, Point3f(6,  7,  8)));
    EXPECT_PTS_CLOSE(p1, cc.WorldToObject(path, Point3f(8, 10, 12)));

    // Two-level path.
    path.push_back(b);

    EXPECT_VECS_CLOSE(Vector3f(0,  0,   0),    cc.ObjectToWorld(path, v0));
    EXPECT_VECS_CLOSE(Vector3f(20, 300, 4000), cc.ObjectToWorld(path, v1));

    EXPECT_PTS_CLOSE(Point3f(48,  73, 100),  cc.ObjectToWorld(path, p0));
    EXPECT_PTS_CLOSE(Point3f(68, 373, 4100), cc.ObjectToWorld(path, p1));

    EXPECT_VECS_CLOSE(v0, cc.WorldToObject(path, Vector3f(0,  0,   0)));
    EXPECT_VECS_CLOSE(v1, cc.WorldToObject(path, Vector3f(20, 300, 4000)));

    EXPECT_PTS_CLOSE(p0, cc.WorldToObject(path, Point3f(48,  73, 100)));
    EXPECT_PTS_CLOSE(p1, cc.WorldToObject(path, Point3f(68, 373, 4100)));
}

TEST_F(CoordConvTest, LocalWorld) {
    SG::NodePtr a = CreateNode("A");
    SG::NodePtr b = CreateNode("B");
    a->SetScale(Vector3f(2, 3, 4));
    b->SetScale(Vector3f(10, 100, 1000));
    a->SetTranslation(Vector3f(6, 7, 8));
    b->SetTranslation(Vector3f(21, 22, 23));

    CoordConv cc;

    // One-level path - transforms are ignored.
    SG::NodePath path;
    path.push_back(a);

    EXPECT_VECS_CLOSE(Vector3f(0, 0, 0), cc.LocalToWorld(path, v0));
    EXPECT_VECS_CLOSE(Vector3f(1, 1, 1), cc.LocalToWorld(path, v1));

    EXPECT_PTS_CLOSE(Point3f(0, 0, 0), cc.LocalToWorld(path, p0));
    EXPECT_PTS_CLOSE(Point3f(1, 1, 1), cc.LocalToWorld(path, p1));

    EXPECT_VECS_CLOSE(v0, cc.WorldToLocal(path, Vector3f(0, 0, 0)));
    EXPECT_VECS_CLOSE(v1, cc.WorldToLocal(path, Vector3f(1, 1, 1)));

    EXPECT_PTS_CLOSE(p0, cc.WorldToLocal(path, Point3f(0,  0,  0)));
    EXPECT_PTS_CLOSE(p1, cc.WorldToLocal(path, Point3f(1,  1,  1)));

    // Two-level path.
    path.push_back(b);

    EXPECT_VECS_CLOSE(Vector3f(0, 0, 0), cc.LocalToWorld(path, v0));
    EXPECT_VECS_CLOSE(Vector3f(2, 3, 4), cc.LocalToWorld(path, v1));

    EXPECT_PTS_CLOSE(Point3f(6,  7,  8), cc.LocalToWorld(path, p0));
    EXPECT_PTS_CLOSE(Point3f(8, 10, 12), cc.LocalToWorld(path, p1));

    EXPECT_VECS_CLOSE(v0, cc.WorldToLocal(path, Vector3f(0, 0, 0)));
    EXPECT_VECS_CLOSE(v1, cc.WorldToLocal(path, Vector3f(2, 3, 4)));

    EXPECT_PTS_CLOSE(p0, cc.WorldToLocal(path, Point3f(6,  7,  8)));
    EXPECT_PTS_CLOSE(p1, cc.WorldToLocal(path, Point3f(8, 10, 12)));
}

TEST_F(CoordConvTest, StageWorld) {
    SG::NodePtr root  = CreateNode("Root");
    SG::NodePtr stage = CreateNode("Stage");

    root->SetScale(Vector3f(2, 3, 4));
    stage->SetScale(Vector3f(100, 200, 300));

    SG::NodePath stage_path;
    stage_path.push_back(root);
    stage_path.push_back(stage);

    CoordConv cc;
    cc.SetStagePath(stage_path);

    // The transforms on the root and stage should both be applied.
    EXPECT_VECS_CLOSE(Vector3f(0,   0,   0),    cc.StageToWorld(v0));
    EXPECT_VECS_CLOSE(Vector3f(200, 600, 1200), cc.StageToWorld(v1));
    EXPECT_PTS_CLOSE(Point3f(0,   0,   0),    cc.StageToWorld(p0));
    EXPECT_PTS_CLOSE(Point3f(200, 600, 1200), cc.StageToWorld(p1));

    EXPECT_VECS_CLOSE(v0, cc.WorldToStage(Vector3f(0,   0,   0)));
    EXPECT_VECS_CLOSE(v1, cc.WorldToStage(Vector3f(200, 600, 1200)));
    EXPECT_PTS_CLOSE(p0, cc.WorldToStage(Point3f(0,   0,   0)));
    EXPECT_PTS_CLOSE(p1, cc.WorldToStage(Point3f(200, 600, 1200)));
}

TEST_F(CoordConvTest, ObjectStage) {
    SG::NodePtr root  = CreateNode("Root");
    SG::NodePtr stage = CreateNode("Stage");

    root->SetScale(Vector3f(20, 30, 40));
    stage->SetScale(Vector3f(100, 200, 300));

    SG::NodePath stage_path;
    stage_path.push_back(root);
    stage_path.push_back(stage);

    CoordConv cc;
    cc.SetStagePath(stage_path);

    // Create a path from the root through the stage to node "A".
    SG::NodePtr a = CreateNode("A");
    a->SetScale(Vector3f(2, 3, 4));
    SG::NodePath path;
    path.push_back(root);
    path.push_back(stage);
    path.push_back(a);

    EXPECT_VECS_CLOSE(Vector3f(0, 0, 0), cc.ObjectToStage(path, v0));
    EXPECT_VECS_CLOSE(Vector3f(2, 3, 4), cc.ObjectToStage(path, v1));

    EXPECT_PTS_CLOSE(Point3f(0, 0, 0), cc.ObjectToStage(path, p0));
    EXPECT_PTS_CLOSE(Point3f(2, 3, 4), cc.ObjectToStage(path, p1));

    EXPECT_VECS_CLOSE(v0, cc.StageToObject(path, Vector3f(0, 0, 0)));
    EXPECT_VECS_CLOSE(v1, cc.StageToObject(path, Vector3f(2, 3, 4)));

    EXPECT_PTS_CLOSE(p0, cc.StageToObject(path, Point3f(0, 0, 0)));
    EXPECT_PTS_CLOSE(p1, cc.StageToObject(path, Point3f(2, 3, 4)));
}

TEST_F(CoordConvTest, LocalStage) {
    SG::NodePtr root  = CreateNode("Root");
    SG::NodePtr stage = CreateNode("Stage");

    root->SetScale(Vector3f(20, 30, 40));
    stage->SetScale(Vector3f(100, 200, 300));

    SG::NodePath stage_path;
    stage_path.push_back(root);
    stage_path.push_back(stage);

    CoordConv cc;
    cc.SetStagePath(stage_path);

    // Create a path from the root through the stage to nodes "A" and "B". Only
    // A's transforms should be included.
    SG::NodePtr a = CreateNode("A");
    SG::NodePtr b = CreateNode("B");
    a->SetScale(Vector3f(2, 3, 4));
    b->SetScale(Vector3f(200, 300, 400));
    SG::NodePath path;
    path.push_back(root);
    path.push_back(stage);
    path.push_back(a);
    path.push_back(b);

    EXPECT_VECS_CLOSE(Vector3f(0, 0, 0), cc.LocalToStage(path, v0));
    EXPECT_VECS_CLOSE(Vector3f(2, 3, 4), cc.LocalToStage(path, v1));

    EXPECT_PTS_CLOSE(Point3f(0, 0, 0), cc.LocalToStage(path, p0));
    EXPECT_PTS_CLOSE(Point3f(2, 3, 4), cc.LocalToStage(path, p1));

    EXPECT_VECS_CLOSE(v0, cc.StageToLocal(path, Vector3f(0, 0, 0)));
    EXPECT_VECS_CLOSE(v1, cc.StageToLocal(path, Vector3f(2, 3, 4)));

    EXPECT_PTS_CLOSE(p0, cc.StageToLocal(path, Point3f(0, 0, 0)));
    EXPECT_PTS_CLOSE(p1, cc.StageToLocal(path, Point3f(2, 3, 4)));
}
