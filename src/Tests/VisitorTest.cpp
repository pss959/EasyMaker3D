#include <string>

#include "SG/Node.h"
#include "SG/Visitor.h"
#include "SceneTestBase.h"

class VisitorTest : public SceneTestBase {
  protected:
    // Accumulated result string.
    std::string result_str;

    // If the name of a visited node matches this string, kPrune will be
    // returned.
    std::string prune_name;

    // If the name of a visited node matches this string, kStop will be
    // returned.
    std::string stop_name;

    // Handy 3-level scene input.
    const std::string scene_3level_input =
        "Scene \"SomeScene\" {\n"
        "  render_passes: [\n"
        "    LightingPass {\n"
        "      root: Node \"Parent\" {\n"
        "        children: [\n"
        "          Node \"AChild\" {},\n"
        "          Node \"SecondChild\" {\n"
        "            children: [\n"
        "              Node \"GrandKid\" {},\n"
        "            ]\n"
        "          },\n"
        "          Node \"Child3\" {},\n"
        "        ]\n"
        "      }\n"
        "    }\n"
        "  ]\n"
        "}\n";

    // Function that concatenates the visited node name to the string.
    SG::Visitor::TraversalCode StrFunc(const SG::NodePath &path) {
        const std::string &name = path.back()->GetName();
        result_str += '.';
        result_str += name;
        if (name == prune_name)
            return SG::Visitor::TraversalCode::kPrune;
        else if (name == stop_name)
            return SG::Visitor::TraversalCode::kStop;
        else
            return SG::Visitor::TraversalCode::kContinue;
    }

    // Reads a scene from a string, visits its root, and returns the resulting
    // code.
    SG::Visitor::TraversalCode VisitScene(const std::string &input) {
        result_str = "";
        SG::ScenePtr scene = ReadScene(input);
        SG::Visitor visitor;
        return visitor.Visit(scene->GetRootNode(),
                             std::bind(&VisitorTest::StrFunc, this,
                                       std::placeholders::_1));
    }
};

TEST_F(VisitorTest, EmptyScene) {
    const std::string input = "Scene {}\n";
    EXPECT_EQ(SG::Visitor::TraversalCode::kContinue, VisitScene(input));
    EXPECT_EQ("", result_str);
}

TEST_F(VisitorTest, FullScene) {
    EXPECT_EQ(SG::Visitor::TraversalCode::kContinue,
              VisitScene(scene_3level_input));
    EXPECT_EQ(".Parent.AChild.SecondChild.GrandKid.Child3", result_str);
}

TEST_F(VisitorTest, Prune) {
    prune_name = "SecondChild";
    EXPECT_EQ(SG::Visitor::TraversalCode::kContinue,
              VisitScene(scene_3level_input));
    EXPECT_EQ(".Parent.AChild.SecondChild.Child3", result_str);

    prune_name = "Child3";
    EXPECT_EQ(SG::Visitor::TraversalCode::kPrune,
              VisitScene(scene_3level_input));
    EXPECT_EQ(".Parent.AChild.SecondChild.GrandKid.Child3", result_str);
}

TEST_F(VisitorTest, Stop) {
    stop_name = "SecondChild";
    EXPECT_EQ(SG::Visitor::TraversalCode::kStop,
              VisitScene(scene_3level_input));
    EXPECT_EQ(".Parent.AChild.SecondChild", result_str);

    stop_name = "Child3";
    EXPECT_EQ(SG::Visitor::TraversalCode::kStop,
              VisitScene(scene_3level_input));
    EXPECT_EQ(".Parent.AChild.SecondChild.GrandKid.Child3", result_str);

    stop_name = "Parent";
    EXPECT_EQ(SG::Visitor::TraversalCode::kStop,
              VisitScene(scene_3level_input));
    EXPECT_EQ(".Parent", result_str);
}
