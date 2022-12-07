#include <string>

#include "App/CoordConv.h"
#include "App/RegisterTypes.h"
#include "Parser/Registry.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

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

    // One-level path.
    SG::NodePath path;
    path.push_back(a);

    {
        CoordConv cc(path);

        EXPECT_VECS_CLOSE(Vector3f(0, 0, 0), cc.ObjectToRoot(v0));
        EXPECT_VECS_CLOSE(Vector3f(2, 3, 4), cc.ObjectToRoot(v1));

        EXPECT_PTS_CLOSE(Point3f(6,  7,  8), cc.ObjectToRoot(p0));
        EXPECT_PTS_CLOSE(Point3f(8, 10, 12), cc.ObjectToRoot(p1));

        EXPECT_VECS_CLOSE(v0, cc.RootToObject(Vector3f(0, 0, 0)));
        EXPECT_VECS_CLOSE(v1, cc.RootToObject(Vector3f(2, 3, 4)));

        EXPECT_PTS_CLOSE(p0, cc.RootToObject(Point3f(6,  7,  8)));
        EXPECT_PTS_CLOSE(p1, cc.RootToObject(Point3f(8, 10, 12)));
    }

    // Two-level path.
    path.push_back(b);

    {
        CoordConv cc(path);

        EXPECT_VECS_CLOSE(Vector3f(0,  0,   0),    cc.ObjectToRoot(v0));
        EXPECT_VECS_CLOSE(Vector3f(20, 300, 4000), cc.ObjectToRoot(v1));

        EXPECT_PTS_CLOSE(Point3f(48,  73, 100),  cc.ObjectToRoot(p0));
        EXPECT_PTS_CLOSE(Point3f(68, 373, 4100), cc.ObjectToRoot(p1));

        EXPECT_VECS_CLOSE(v0, cc.RootToObject(Vector3f(0,  0,   0)));
        EXPECT_VECS_CLOSE(v1, cc.RootToObject(Vector3f(20, 300, 4000)));

        EXPECT_PTS_CLOSE(p0, cc.RootToObject(Point3f(48,  73, 100)));
        EXPECT_PTS_CLOSE(p1, cc.RootToObject(Point3f(68, 373, 4100)));
    }
}

TEST_F(CoordConvTest, LocalWorld) {
    SG::NodePtr a = CreateNode("A");
    SG::NodePtr b = CreateNode("B");
    a->SetScale(Vector3f(2, 3, 4));
    b->SetScale(Vector3f(10, 100, 1000));
    a->SetTranslation(Vector3f(6, 7, 8));
    b->SetTranslation(Vector3f(21, 22, 23));

    // One-level path - transforms are ignored.
    SG::NodePath path;
    path.push_back(a);

    {
        CoordConv cc(path);

        EXPECT_VECS_CLOSE(Vector3f(0, 0, 0), cc.LocalToRoot(v0));
        EXPECT_VECS_CLOSE(Vector3f(1, 1, 1), cc.LocalToRoot(v1));

        EXPECT_PTS_CLOSE(Point3f(0, 0, 0), cc.LocalToRoot(p0));
        EXPECT_PTS_CLOSE(Point3f(1, 1, 1), cc.LocalToRoot(p1));

        EXPECT_VECS_CLOSE(v0, cc.RootToLocal(Vector3f(0, 0, 0)));
        EXPECT_VECS_CLOSE(v1, cc.RootToLocal(Vector3f(1, 1, 1)));

        EXPECT_PTS_CLOSE(p0, cc.RootToLocal(Point3f(0,  0,  0)));
        EXPECT_PTS_CLOSE(p1, cc.RootToLocal(Point3f(1,  1,  1)));
    }

    // Two-level path.
    path.push_back(b);

    {
        CoordConv cc(path);

        EXPECT_VECS_CLOSE(Vector3f(0, 0, 0), cc.LocalToRoot(v0));
        EXPECT_VECS_CLOSE(Vector3f(2, 3, 4), cc.LocalToRoot(v1));

        EXPECT_PTS_CLOSE(Point3f(6,  7,  8), cc.LocalToRoot(p0));
        EXPECT_PTS_CLOSE(Point3f(8, 10, 12), cc.LocalToRoot(p1));

        EXPECT_VECS_CLOSE(v0, cc.RootToLocal(Vector3f(0, 0, 0)));
        EXPECT_VECS_CLOSE(v1, cc.RootToLocal(Vector3f(2, 3, 4)));

        EXPECT_PTS_CLOSE(p0, cc.RootToLocal(Point3f(6,  7,  8)));
        EXPECT_PTS_CLOSE(p1, cc.RootToLocal(Point3f(8, 10, 12)));
    }
}
