#include <sstream>
#include <string>

#include <ion/gfxutils/printer.h>

#include "Parser/Exception.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "SceneTestBase.h"

class SearchTest : public SceneTestBase {
};

TEST_F(SearchTest, Empty) {
    std::string input = "Scene {}";
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene);
    EXPECT_NULL(scene->GetRootNode());

    EXPECT_NULL(SG::FindNodeInScene(*scene, "Anything"));
    EXPECT_TRUE(SG::FindNodePathInScene(*scene, "Anything").empty());
}

TEST_F(SearchTest, TwoLevel) {
    std::string input =
        "Scene \"MyScene\" {\n"
        "  root: Node {\n"
        "    children: [\n"
        "      Node \"SomeChild\" {}\n"
        "    ]\n"
        "  }\n"
        "}\n";
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene);

    SG::NodePtr  node = SG::FindNodeInScene(*scene, "SomeChild");
    EXPECT_EQ("SomeChild", node->GetName());

    SG::NodePath path = SG::FindNodePathInScene(*scene, "SomeChild");
    EXPECT_FALSE(path.empty());
    EXPECT_EQ(2U, path.size());
    EXPECT_EQ(scene->GetRootNode(), path[0]);
    EXPECT_EQ(node,                 path[1]);

    EXPECT_NULL(SG::FindNodeInScene(*scene, "NotFound"));
    EXPECT_TRUE(SG::FindNodePathInScene(*scene, "NotFound").empty());
}

TEST_F(SearchTest, MultiLevel) {
    std::string input =
        "Scene {\n"
        "  root: Node \"Level0\" {\n"
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
        "    ]\n"
        "  }\n"
        "}\n";
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene);

    SG::NodePath path = SG::FindNodePathInScene(*scene, "Level1b");
    EXPECT_EQ(2U, path.size());
    EXPECT_EQ("Level0",  path[0]->GetName());
    EXPECT_EQ("Level1b", path[1]->GetName());

    path = SG::FindNodePathInScene(*scene, "Level2b");
    EXPECT_EQ(3U, path.size());
    EXPECT_EQ("Level0",  path[0]->GetName());
    EXPECT_EQ("Level1a", path[1]->GetName());
    EXPECT_EQ("Level2b", path[2]->GetName());

    path = SG::FindNodePathInScene(*scene, "Level2d");
    EXPECT_EQ(3U, path.size());
    EXPECT_EQ("Level0",  path[0]->GetName());
    EXPECT_EQ("Level1b", path[1]->GetName());
    EXPECT_EQ("Level2d", path[2]->GetName());

    SG::NodePtr node = SG::FindNodeInScene(*scene, "Level1b");
    EXPECT_NOT_NULL(node);
    path = SG::FindNodePathUnderNode(node, "Level2c");
    EXPECT_EQ(2U, path.size());
    EXPECT_EQ("Level1b", path[0]->GetName());
    EXPECT_EQ("Level2c", path[1]->GetName());
}
