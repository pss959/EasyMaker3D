#include <sstream>
#include <string>

#include "Parser/Exception.h"
#include "SG/Box.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/Assert.h"

class SearchTest : public SceneTestBase {};

TEST_F(SearchTest, EmptyScene) {
    auto scene = CreateObject<SG::Scene>();
    auto node  = CreateObject<SG::Node>();

    EXPECT_TRUE(SG::FindNodePathInScene(*scene, *node).empty());
    EXPECT_TRUE(SG::FindNodePathInScene(*scene, "NotFound", true).empty());

    TEST_THROW(SG::FindNodePathInScene(*scene, "Anything"),
               AssertException, "not found");
}

TEST_F(SearchTest, OneLevel) {
    const std::string input = "Scene {}";
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene);
    EXPECT_NULL(scene->GetRootNode());

    EXPECT_NULL(SG::FindNodeInScene(*scene, "Anything", true));
    EXPECT_TRUE(SG::FindNodePathInScene(*scene, "Anything", true).empty());
}

TEST_F(SearchTest, TwoLevel) {
    SG::ScenePtr scene =
        BuildAndReadScene("children: [ Node \"SomeChild\" {} ] ");
    EXPECT_NOT_NULL(scene);

    SG::NodePtr node = SG::FindNodeInScene(*scene, "SomeChild", true);
    EXPECT_EQ("SomeChild", node->GetName());

    SG::NodePath path = SG::FindNodePathInScene(*scene, *node);
    EXPECT_FALSE(path.empty());
    EXPECT_EQ(2U, path.size());
    EXPECT_EQ(scene->GetRootNode(), path[0]);
    EXPECT_EQ(node,                 path[1]);

    path = SG::FindNodePathInScene(*scene, "SomeChild", true);
    EXPECT_FALSE(path.empty());
    EXPECT_EQ(2U, path.size());
    EXPECT_EQ(scene->GetRootNode(), path[0]);
    EXPECT_EQ(node,                 path[1]);

    EXPECT_NULL(SG::FindNodeInScene(*scene, "NotFound", true));
    EXPECT_TRUE(SG::FindNodePathInScene(*scene, "NotFound", true).empty());
}

TEST_F(SearchTest, MultiLevel) {
    const std::string contents = R"(
  children: [
    Node "Level1a" {
      children: [
        Node "Level2a" {}
        Node "Level2b" {}
      ]
    },
    TextNode "Level1b" {
      children: [
        Node     "Level2c" {}
        TextNode "Level2d" {}
      ]
    },
    Node "Level1c" {
      disabled_flags: "kSearch",
      children: [
        Node "Level2e" {}
        Node "Level2f" {}
      ]
    },
  ]
)";
    SG::ScenePtr scene = BuildAndReadScene(contents);
    EXPECT_NOT_NULL(scene);

    SG::NodePath path = SG::FindNodePathInScene(*scene, "Level1b");
    EXPECT_EQ(2U, path.size());
    EXPECT_EQ("Root",    path[0]->GetName());
    EXPECT_EQ("Level1b", path[1]->GetName());

    path = SG::FindNodePathInScene(*scene, "Level2b");
    EXPECT_EQ(3U, path.size());
    EXPECT_EQ("Root",    path[0]->GetName());
    EXPECT_EQ("Level1a", path[1]->GetName());
    EXPECT_EQ("Level2b", path[2]->GetName());

    path = SG::FindNodePathInScene(*scene, "Level2d");
    EXPECT_EQ(3U, path.size());
    EXPECT_EQ("Root",    path[0]->GetName());
    EXPECT_EQ("Level1b", path[1]->GetName());
    EXPECT_EQ("Level2d", path[2]->GetName());

    SG::NodePtr node = SG::FindNodeInScene(*scene, "Level1b");
    EXPECT_NOT_NULL(node);
    path = SG::FindNodePathUnderNode(node, "Level2c");
    EXPECT_EQ(2U, path.size());
    EXPECT_EQ("Level1b", path[0]->GetName());
    EXPECT_EQ("Level2c", path[1]->GetName());

    // Typed interface.
    auto l1b = SG::FindTypedNodeInScene<SG::TextNode>(*scene,  "Level1b");
    auto l2d = SG::FindTypedNodeUnderNode<SG::TextNode>(*node, "Level2d");
    EXPECT_EQ("Level1b", l1b->GetName());
    EXPECT_EQ("Level2d", l2d->GetName());

    auto tn = SG::FindFirstTypedNodeUnderNode(*scene->GetRootNode(),
                                              "TextNode");
    EXPECT_NOT_NULL(tn);
    EXPECT_EQ("Level1b", tn->GetName());

    // Test non-asserting error cases. Some need ok_if_not_found = true.
    auto bad_node = CreateObject<SG::Node>();
    bad_node->SetFlagEnabled(SG::Node::Flag::kSearch, false);
    EXPECT_TRUE(SG::FindNodePathUnderNode(bad_node, *node).empty());
    EXPECT_TRUE(SG::FindNodePathUnderNode(node, *bad_node).empty());
    EXPECT_NULL(SG::FindNodeUnderNode(*node,    "NotFound", true));
    EXPECT_TRUE(SG::FindNodePathUnderNode(node, "NotFound", true).empty());

    // Wrong type.
    TEST_THROW(SG::FindTypedNodeInScene<SG::TextNode>(*scene, "Level1a"),
               AssertException, "Typed Node");
    TEST_THROW(SG::FindTypedNodeUnderNode<SG::TextNode>(*node, "Level2c"),
               AssertException, "Typed Node");

    // Search fails because of disabled flag.
    TEST_THROW(SG::FindNodeInScene(*scene, "Level1c"),
               AssertException, "not found");
    TEST_THROW(SG::FindNodeInScene(*scene, "Level2e"),
               AssertException, "not found");
}

TEST_F(SearchTest, FindNodes) {
    const std::string contents = R"(
  children: [
    Node "FindMe0" {}
    Node "FindMe1" {
      children: [
        Node "Blah" {}
        Node "FindMe2" {}
      ]
    },
    TextNode "Foo" {
      children: [
        Node "FindMe3" {}
        Node "FindMe4" {}
        USE "FindMe0"
      ]
    },
    Node "Disabled" {  # None of these should be found.
      disabled_flags: "kSearch",
      children: [
        Node "FindMe5" {}
        Node "FindMe6" {}
        USE "FindMe0"
      ]
    },
  ]
)";
    SG::ScenePtr scene = BuildAndReadScene(contents);
    EXPECT_NOT_NULL(scene);

    auto func1 = [](const SG::Node &node){
        return node.GetName().starts_with("FindMe");
    };
    const std::vector<SG::NodePtr> nodes1a =
        SG::FindNodes(scene->GetRootNode(), func1);
    EXPECT_EQ(6U, nodes1a.size());
    EXPECT_EQ("FindMe0", nodes1a[0]->GetName());
    EXPECT_EQ("FindMe1", nodes1a[1]->GetName());
    EXPECT_EQ("FindMe2", nodes1a[2]->GetName());
    EXPECT_EQ("FindMe3", nodes1a[3]->GetName());
    EXPECT_EQ("FindMe4", nodes1a[4]->GetName());
    EXPECT_EQ("FindMe0", nodes1a[5]->GetName());
    const std::vector<SG::NodePtr> nodes1b =
        SG::FindUniqueNodes(scene->GetRootNode(), func1);
    EXPECT_EQ(5U, nodes1b.size());
    EXPECT_EQ("FindMe0", nodes1b[0]->GetName());
    EXPECT_EQ("FindMe1", nodes1b[1]->GetName());
    EXPECT_EQ("FindMe2", nodes1b[2]->GetName());
    EXPECT_EQ("FindMe3", nodes1b[3]->GetName());
    EXPECT_EQ("FindMe4", nodes1b[4]->GetName());

    auto func2 = [](const SG::Node &node){
        return node.GetName().starts_with("NoSuchName");
    };
    const std::vector<SG::NodePtr> nodes2 =
        SG::FindNodes(scene->GetRootNode(), func2);
    EXPECT_EQ(0U, nodes2.size());

    // Test FindNodePaths().
    const auto paths = SG::FindNodePaths(scene->GetRootNode(), func1);
    EXPECT_EQ(6U, paths.size());
    EXPECT_EQ("FindMe0", paths[0].back()->GetName());
    EXPECT_EQ("FindMe1", paths[1].back()->GetName());
    EXPECT_EQ("FindMe2", paths[2].back()->GetName());
    EXPECT_EQ("FindMe3", paths[3].back()->GetName());
    EXPECT_EQ("FindMe4", paths[4].back()->GetName());
    EXPECT_EQ("FindMe0", paths[5].back()->GetName());
}

TEST_F(SearchTest, FindShape) {
    const std::string contents = R"(
  children: [
    Node "N" {
      shapes: [
        Box      "BoxShape" {}
        Cylinder "CylShape" {}
      ]
    },
  ]
)";
    SG::ScenePtr scene = BuildAndReadScene(contents);
    EXPECT_NOT_NULL(scene);

    auto node = SG::FindNodeInScene(*scene, "N");
    EXPECT_NOT_NULL(node);

    auto shape = SG::FindShapeInNode(*node, "BoxShape");
    EXPECT_NOT_NULL(shape);
    EXPECT_EQ("BoxShape", shape->GetName());

    auto box = SG::FindTypedShapeInNode<SG::Box>(*node, "BoxShape");
    EXPECT_NOT_NULL(box);
    EXPECT_EQ("BoxShape", box->GetName());

    // Errors.
    EXPECT_NULL(SG::FindShapeInNode(*node, "NoSuchShape"));
    TEST_THROW(SG::FindTypedShapeInNode<SG::Box>(*node, "CylShape"),
               AssertException, "Typed Shape");
}

TEST_F(SearchTest, AssertErrors) {
    SG::ScenePtr scene = BuildAndReadScene("");
    EXPECT_NOT_NULL(scene);
    EXPECT_NOT_NULL(scene->GetRootNode());

    TEST_THROW(SG::FindNodeInScene(*scene, "Anything"),
               AssertException, "not found");
    TEST_THROW(SG::FindNodePathInScene(*scene, "Anything"),
               AssertException, "not found");
    TEST_THROW(SG::FindNodePathUnderNode(scene->GetRootNode(), "Anything"),
               AssertException, "not found under node");
    TEST_THROW(SG::FindNodeUnderNode(*scene->GetRootNode(), "Anything"),
               AssertException, "not found under node");
}
