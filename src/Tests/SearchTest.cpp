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
    std::string input = "Scene {}";
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene);
    EXPECT_NULL(scene->GetRootNode());

    EXPECT_NULL(SG::FindNodeInScene(*scene, "Anything", true));
    EXPECT_TRUE(SG::FindNodePathInScene(*scene, "Anything", true).empty());
}

TEST_F(SearchTest, TwoLevel) {
    std::string input = str1 + "children: [ Node \"SomeChild\" {} ] " + str2;
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
    std::string input = str1 +
        "    children: [\n"
        "      Node \"Level1a\" {\n"
        "        children: [\n"
        "          Node \"Level2a\" {}\n"
        "          Node \"Level2b\" {}\n"
        "        ]\n"
        "      },\n"
        "      Node \"Level1b\" {\n"
        "        children: [\n"
        "          Node \"Level2c\" {}\n"
        "          Node \"Level2d\" {}\n"
        "        ]\n"
        "      },\n"
        "      Node \"Level1c\" {\n"
        "        disabled_flags: \"kSearch\",\n"
        "        children: [\n"
        "          Node \"Level2e\" {}\n"
        "          Node \"Level2f\" {}\n"
        "        ]\n"
        "      },\n"
        "    ]\n" + str2;
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
    std::string input = str1 +
        "    children: [\n"
        "      Node \"FindMe0\" {}\n"
        "      Node \"FindMe1\" {\n"
        "        children: [\n"
        "          Node \"Blah\" {}\n"
        "          Node \"FindMe2\" {}\n"
        "        ]\n"
        "      },\n"
        "      Node \"Foo\" {\n"
        "        children: [\n"
        "          Node \"FindMe3\" {}\n"
        "          Node \"FindMe4\" {}\n"
        "          USE \"FindMe0\"\n"
        "        ]\n"
        "      },\n"
        "      Node \"Disabled\" {\n"  // None of these should be found.
        "        disabled_flags: \"kSearch\",\n"
        "        children: [\n"
        "          Node \"FindMe5\" {}\n"
        "          Node \"FindMe6\" {}\n"
        "          USE \"FindMe0\"\n"
        "        ]\n"
        "      },\n"
        "    ]\n" + str2;
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
    std::string input = str1 + str2;
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
