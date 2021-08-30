#include <string>

#include "SG/Node.h"
#include "SG/Visitor.h"
#include "SceneTestBase.h"

// Derived Visitor that allows testing.
class TVisitor : public SG::Visitor {
  public:
    // Accumulated result string.
    std::string result_str;

    // If the name of a visited node matches this string, kPrune will be
    // returned.
    std::string prune_name;

    // If the name of a visited node matches this string, kStop will be
    // returned.
    std::string stop_name;

    // If this string is not empty, the current Visitor node path is matched
    // against it when the stop_name is reached.
    std::string stop_node_path;

  protected:
    // Concatenates the visited node name to the string.
    virtual TraversalCode VisitNodeStart(const SG::NodePath &path) override {
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

    virtual void VisitNodeEnd(const SG::NodePath &path) override {
        const std::string &name = path.back()->GetName();
        if (name == stop_name && ! stop_node_path.empty()) {
            EXPECT_EQ(stop_node_path, path.ToString());
        }
    }
};

class VisitorTest : public SceneTestBase {
  protected:
    TVisitor visitor;

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

    // Reads a scene from a string, visits its root, and returns the resulting
    // code.
    SG::Visitor::TraversalCode VisitScene(const std::string &input) {
        visitor.result_str = "";
        SG::ScenePtr scene = ReadScene(input);
        return visitor.Visit(scene->GetRootNode());
    }
};

TEST_F(VisitorTest, EmptyScene) {
    const std::string input = "Scene {}\n";
    EXPECT_EQ(SG::Visitor::TraversalCode::kContinue, VisitScene(input));
    EXPECT_EQ("", visitor.result_str);
}

TEST_F(VisitorTest, FullScene) {
    EXPECT_EQ(SG::Visitor::TraversalCode::kContinue,
              VisitScene(scene_3level_input));
    EXPECT_EQ(".Parent.AChild.SecondChild.GrandKid.Child3", visitor.result_str);
}

TEST_F(VisitorTest, Prune) {
    visitor.prune_name = "SecondChild";
    EXPECT_EQ(SG::Visitor::TraversalCode::kContinue,
              VisitScene(scene_3level_input));
    EXPECT_EQ(".Parent.AChild.SecondChild.Child3", visitor.result_str);

    visitor.prune_name = "Child3";
    EXPECT_EQ(SG::Visitor::TraversalCode::kPrune,
              VisitScene(scene_3level_input));
    EXPECT_EQ(".Parent.AChild.SecondChild.GrandKid.Child3", visitor.result_str);
}

TEST_F(VisitorTest, Stop) {
    visitor.stop_name = "SecondChild";
    EXPECT_EQ(SG::Visitor::TraversalCode::kStop,
              VisitScene(scene_3level_input));
    EXPECT_EQ(".Parent.AChild.SecondChild", visitor.result_str);

    visitor.stop_name = "Child3";
    EXPECT_EQ(SG::Visitor::TraversalCode::kStop,
              VisitScene(scene_3level_input));
    EXPECT_EQ(".Parent.AChild.SecondChild.GrandKid.Child3", visitor.result_str);

    visitor.stop_name = "Parent";
    EXPECT_EQ(SG::Visitor::TraversalCode::kStop,
              VisitScene(scene_3level_input));
    EXPECT_EQ(".Parent", visitor.result_str);
}

TEST_F(VisitorTest, Path) {
    visitor.stop_name = "Child3";
    visitor.stop_node_path = "<Parent/Child3>";  // Expected path at Child3.
    EXPECT_EQ(SG::Visitor::TraversalCode::kStop,
              VisitScene(scene_3level_input));
    EXPECT_EQ(".Parent.AChild.SecondChild.GrandKid.Child3", visitor.result_str);
}
