#include <string>

#include "Parser/Registry.h"
#include "RegisterTypes.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "TestBase.h"
#include "Testing.h"

class NodePathTest : public TestBase {
  public:
    NodePathTest()  { RegisterTypes(); }
    ~NodePathTest() { UnregisterTypes(); }

    // Creates and returns a named Node to add to a NodePath.
    SG::NodePtr CreateNode(const std::string &name) {
        return CreateObject<SG::Node>(name);
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

TEST_F(NodePathTest, GetEndSubPath) {
    SG::NodePath path;
    path.push_back(CreateNode("A"));
    path.push_back(CreateNode("B"));
    path.push_back(CreateNode("C"));
    path.push_back(CreateNode("D"));

    SG::NodePtr other = CreateNode("Blah");

    EXPECT_EQ("<A/B/C/D>", path.ToString());
    EXPECT_EQ("<A/B/C/D>", path.GetEndSubPath(*path[0]).ToString());
    EXPECT_EQ("<B/C/D>",   path.GetEndSubPath(*path[1]).ToString());
    EXPECT_EQ("<C/D>",     path.GetEndSubPath(*path[2]).ToString());
    EXPECT_EQ("<D>",       path.GetEndSubPath(*path[3]).ToString());

    TEST_THROW(path.GetEndSubPath(*other), std::exception, "Assertion failed");
}

TEST_F(NodePathTest, Stitch) {
    SG::NodePath p0;
    p0.push_back(CreateNode("A"));
    p0.push_back(CreateNode("B"));
    p0.push_back(CreateNode("C"));
    p0.push_back(CreateNode("D"));

    SG::NodePath p1;
    p1.push_back(p0.back());
    p1.push_back(CreateNode("E"));

    SG::NodePath p2;
    p2.push_back(CreateNode("E"));
    p2.push_back(CreateNode("F"));

    SG::NodePath empty;

    // Success cases:
    EXPECT_EQ("<A/B/C/D/E>", SG::NodePath::Stitch(p0, p1).ToString());
    p1.push_back(CreateNode("F"));
    EXPECT_EQ("<A/B/C/D/E/F>", SG::NodePath::Stitch(p0, p1).ToString());

    // Either path empty is bad.
    TEST_THROW(SG::NodePath::Stitch(empty, empty),
               std::exception, "Assertion failed");
    TEST_THROW(SG::NodePath::Stitch(p0, empty),
               std::exception, "Assertion failed");
    TEST_THROW(SG::NodePath::Stitch(empty, p1),
               std::exception, "Assertion failed");

    // So is paths not having last/first node in common.
    TEST_THROW(SG::NodePath::Stitch(p0, p2),
               std::exception, "Assertion failed");
}

TEST_F(NodePathTest, ContainsNode) {
    SG::NodePath path;
    path.push_back(CreateNode("A"));
    path.push_back(CreateNode("B"));
    path.push_back(CreateNode("C"));
    path.push_back(CreateNode("D"));

    SG::NodePtr other = CreateNode("Blah");

    EXPECT_TRUE(path.ContainsNode(*path[0]));
    EXPECT_TRUE(path.ContainsNode(*path[1]));
    EXPECT_TRUE(path.ContainsNode(*path[2]));
    EXPECT_TRUE(path.ContainsNode(*path[3]));
    EXPECT_FALSE(path.ContainsNode(*other));
}

TEST_F(NodePathTest, FindNodeUpwards) {
    SG::NodePtr a = CreateNode("A");
    SG::NodePtr b = CreateNode("B");
    SG::NodePtr c = CreateNode("C");
    SG::NodePath path;
    path.push_back(a);
    path.push_back(b);
    path.push_back(c);

    EXPECT_EQ(a, path.FindNodeUpwards(
                  [](const SG::Node &n){ return n.GetName() == "A"; }));
    EXPECT_EQ(b, path.FindNodeUpwards(
                  [](const SG::Node &n){ return n.GetName() == "B"; }));
    EXPECT_EQ(c, path.FindNodeUpwards(
                  [](const SG::Node &n){ return n.GetName() == "C"; }));
}
