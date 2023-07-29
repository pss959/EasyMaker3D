#include <string>

#include "Parser/Registry.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "SG/TextNode.h"
#include "SG/UnscopedNode.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

class NodePathTest : public TestBaseWithTypes {};

TEST_F(NodePathTest, Default) {
    SG::NodePath path;
    EXPECT_TRUE(path.empty());
}

TEST_F(NodePathTest, GetSubPath) {
    SG::NodePath path;
    path.push_back(CreateObject<SG::Node>("A"));
    path.push_back(CreateObject<SG::Node>("B"));
    path.push_back(CreateObject<SG::Node>("C"));
    path.push_back(CreateObject<SG::Node>("D"));

    SG::NodePtr other = CreateObject<SG::Node>("Blah");

    EXPECT_EQ("<A/B/C/D>", path.ToString());
    EXPECT_EQ("<A/B/C/D>", path.GetSubPath(*path[3]).ToString());
    EXPECT_EQ("<A/B/C>",   path.GetSubPath(*path[2]).ToString());
    EXPECT_EQ("<A/B>",     path.GetSubPath(*path[1]).ToString());
    EXPECT_EQ("<A>",       path.GetSubPath(*path[0]).ToString());

    TEST_THROW(path.GetSubPath(*other), std::exception, "Assertion failed");
}

TEST_F(NodePathTest, GetEndSubPath) {
    SG::NodePath path;
    path.push_back(CreateObject<SG::Node>("A"));
    path.push_back(CreateObject<SG::Node>("B"));
    path.push_back(CreateObject<SG::Node>("C"));
    path.push_back(CreateObject<SG::Node>("D"));

    SG::NodePtr other = CreateObject<SG::Node>("Blah");

    EXPECT_EQ("<A/B/C/D>", path.ToString());
    EXPECT_EQ("<A/B/C/D>", path.GetEndSubPath(*path[0]).ToString());
    EXPECT_EQ("<B/C/D>",   path.GetEndSubPath(*path[1]).ToString());
    EXPECT_EQ("<C/D>",     path.GetEndSubPath(*path[2]).ToString());
    EXPECT_EQ("<D>",       path.GetEndSubPath(*path[3]).ToString());

    TEST_THROW(path.GetEndSubPath(*other), std::exception, "Assertion failed");
}

TEST_F(NodePathTest, Stitch) {
    SG::NodePath p0;
    p0.push_back(CreateObject<SG::Node>("A"));
    p0.push_back(CreateObject<SG::Node>("B"));
    p0.push_back(CreateObject<SG::Node>("C"));
    p0.push_back(CreateObject<SG::Node>("D"));

    SG::NodePath p1;
    p1.push_back(p0.back());
    p1.push_back(CreateObject<SG::Node>("E"));

    SG::NodePath p2;
    p2.push_back(CreateObject<SG::Node>("E"));
    p2.push_back(CreateObject<SG::Node>("F"));

    SG::NodePath empty;

    // Success cases:
    EXPECT_EQ("<A/B/C/D/E>", SG::NodePath::Stitch(p0, p1).ToString());
    p1.push_back(CreateObject<SG::Node>("F"));
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
    path.push_back(CreateObject<SG::Node>("A"));
    path.push_back(CreateObject<SG::Node>("B"));
    path.push_back(CreateObject<SG::Node>("C"));
    path.push_back(CreateObject<SG::Node>("D"));

    SG::NodePtr other = CreateObject<SG::Node>("Blah");

    EXPECT_TRUE(path.ContainsNode(*path[0]));
    EXPECT_TRUE(path.ContainsNode(*path[1]));
    EXPECT_TRUE(path.ContainsNode(*path[2]));
    EXPECT_TRUE(path.ContainsNode(*path[3]));
    EXPECT_FALSE(path.ContainsNode(*other));
}

TEST_F(NodePathTest, FindNodeUpwards) {
    SG::NodePtr a = CreateObject<SG::Node>("A");
    SG::NodePtr b = CreateObject<SG::TextNode>("B");
    SG::NodePtr c = CreateObject<SG::Node>("C");
    SG::NodePath path;
    path.push_back(a);
    path.push_back(b);
    path.push_back(c);

    // Find by name.
    EXPECT_EQ(a, path.FindNodeUpwards(
                  [](const SG::Node &n){ return n.GetName() == "A"; }));
    EXPECT_EQ(b, path.FindNodeUpwards(
                  [](const SG::Node &n){ return n.GetName() == "B"; }));
    EXPECT_EQ(c, path.FindNodeUpwards(
                  [](const SG::Node &n){ return n.GetName() == "C"; }));

    // Find by type.
    EXPECT_EQ(b, path.FindNodeUpwards<SG::TextNode>());

    // Not found.
    EXPECT_NULL(path.FindNodeUpwards(
                  [](const SG::Node &n){ return n.GetName() == "D"; }));
    EXPECT_NULL(path.FindNodeUpwards<SG::UnscopedNode>());
}

TEST_F(NodePathTest, ToString) {
    SG::NodePtr a = CreateObject<SG::Node>("A");
    SG::NodePtr b = CreateObject<SG::TextNode>("B");
    SG::NodePath path;

    EXPECT_EQ("<EMPTY>", path.ToString());
    EXPECT_EQ("<EMPTY>", path.ToString(true));

    path.push_back(a);
    EXPECT_EQ("<A>",       path.ToString());
    EXPECT_EQ("<A(Node)>", path.ToString(true));

    path.push_back(b);
    EXPECT_EQ("<A/B>",                 path.ToString());
    EXPECT_EQ("<A(Node)/B(TextNode)>", path.ToString(true));
}
