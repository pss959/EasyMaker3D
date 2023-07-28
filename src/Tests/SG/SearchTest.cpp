#include <sstream>
#include <string>

#include "Parser/Exception.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Tests/SceneTestBase.h"
#include "Util/Assert.h"

class SearchTest : public SceneTestBase {
};

TEST_F(SearchTest, Empty) {
    const std::string input = "Scene {}";
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene);
    EXPECT_NULL(scene->GetRootNode());

    EXPECT_NULL(SG::FindNodeInScene(*scene, "Anything", true));
    EXPECT_TRUE(SG::FindNodePathInScene(*scene, "Anything", true).empty());
}

TEST_F(SearchTest, TwoLevel) {
    const std::string input =
        BuildSceneString("children: [ Node \"SomeChild\" {} ] ");
    SG::ScenePtr scene = ReadScene(input);
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
    Node "Level1b" {
      children: [
        Node "Level2c" {}
        Node "Level2d" {}
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
    const std::string input = BuildSceneString(contents);
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene);

    SG::NodePath path = SG::FindNodePathInScene(*scene, "Level1b");
    EXPECT_EQ(2U, path.size());
    EXPECT_EQ("Root",   path[0]->GetName());
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

    // Disabled search flag.
    TEST_THROW(SG::FindNodeInScene(*scene, "Level1c"),
               std::exception, "Assertion failed");
    TEST_THROW(SG::FindNodeInScene(*scene, "Level2e"),
               std::exception, "Assertion failed");
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
    Node "Foo" {
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
    const std::string input = BuildSceneString(contents);
    SG::ScenePtr scene = ReadScene(input);
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
}

TEST_F(SearchTest, AssertErrors) {
    const std::string input = BuildSceneString("");
    SG::ScenePtr scene = ReadScene(input);
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
