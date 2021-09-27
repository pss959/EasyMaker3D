#include <string>

#include "SG/Node.h"
#include "SG/NodePath.h"
#include "Testing.h"
#include "RegisterTypes.h"

class NodePathTest : public TestBase {
  public:
    NodePathTest()  { RegisterTypes(); }
    ~NodePathTest() { UnregisterTypes(); }

    // Creates and returns a named Node to add to a NodePath.
    SG::NodePtr CreateNode(const std::string &name) {
        return SG::Node::Create(name);
    }
};

TEST_F(NodePathTest, GetSubPath) {
    SG::NodePath path;
    path.push_back(CreateNode("A"));
    path.push_back(CreateNode("B"));
    path.push_back(CreateNode("C"));
    path.push_back(CreateNode("D"));

    SG::NodePtr other = CreateNode("Blah");

    EXPECT_EQ("<A/B/C/D>", path.ToString());
    EXPECT_EQ("<A/B/C/D>", path.GetSubPath(*path[3]).ToString());
    EXPECT_EQ("<A/B/C>",   path.GetSubPath(*path[2]).ToString());
    EXPECT_EQ("<A/B>",     path.GetSubPath(*path[1]).ToString());
    EXPECT_EQ("<A>",       path.GetSubPath(*path[0]).ToString());

    TEST_THROW(path.GetSubPath(*other), std::exception, "Assertion failed");
}

TEST_F(NodePathTest, FromToLocal) {
    // Set up a path with transformations.

    SG::NodePtr a = CreateNode("A");
    SG::NodePtr b = CreateNode("B");
    a->SetScale(Vector3f(2, 3, 4));
    b->SetScale(Vector3f(20, 30, 40));
    a->SetTranslation(Vector3f(10, 20, 30));
    b->SetTranslation(Vector3f(100, 200, 300));

    SG::NodePath path;
    path.push_back(a);
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 0),
                      path.FromLocal(Vector3f(0, 0, 0)));
    EXPECT_VECS_CLOSE(Vector3f(2, 3, 4),
                      path.FromLocal(Vector3f(1, 1, 1)));
    EXPECT_PTS_CLOSE(Point3f(10, 20, 30),
                     path.FromLocal(Point3f(0, 0, 0)));
    EXPECT_PTS_CLOSE(Point3f(12, 23, 34),
                     path.FromLocal(Point3f(1, 1, 1)));
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 0),
                      path.ToLocal(Vector3f(0, 0, 0)));
    EXPECT_VECS_CLOSE(Vector3f(1, 1, 1),
                      path.ToLocal(Vector3f(2, 3, 4)));
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),
                     path.ToLocal(Point3f(10, 20, 30)));
    EXPECT_PTS_CLOSE(Point3f(1, 1, 1),
                     path.ToLocal(Point3f(12, 23, 34)));

    path.push_back(b);
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 0),
                      path.FromLocal(Vector3f(0, 0, 0)));
    EXPECT_VECS_CLOSE(Vector3f(40, 90, 160),
                      path.FromLocal(Vector3f(1, 1, 1)));
    EXPECT_PTS_CLOSE(Point3f(210, 620, 1230),
                     path.FromLocal(Point3f(0, 0, 0)));
    EXPECT_PTS_CLOSE(Point3f(250, 710, 1390),
                     path.FromLocal(Point3f(1, 1, 1)));
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 0),
                      path.ToLocal(Vector3f(0, 0, 0)));
    EXPECT_VECS_CLOSE(Vector3f(1, 1, 1),
                      path.ToLocal(Vector3f(40, 90, 160)));
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),
                     path.ToLocal(Point3f(210, 620, 1230)));
    EXPECT_PTS_CLOSE(Point3f(1, 1, 1),
                     path.ToLocal(Point3f(250, 710, 1390)));
}
