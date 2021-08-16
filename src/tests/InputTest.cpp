#include <sstream>
#include <string>

#include <sstream>

#include "Graph/Node.h"
#include "Graph/Scene.h"
#include "Input/Reader.h"
#include "Input/Tracker.h"
#include "Testing.h"
#include "Util/String.h"

#include <ion/gfx/node.h>
#include <ion/gfxutils/printer.h>
#include <ion/gfxutils/shadermanager.h>
#include <ion/math/matrix.h>
#include <ion/math/matrixutils.h>

using ion::gfx::NodePtr;
using ion::math::Matrix4f;

// Tests that a Input::Exception is thrown and that its message contains
// the given string pattern.
#define TEST_THROW_(STMT, PATTERN) TEST_THROW(STMT, Input::Exception, PATTERN)

class InputTest : public TestBase {
 protected:
    InputTest() :
        shader_manager(new ion::gfxutils::ShaderManager),
        reader(tracker, *shader_manager) {}

    // Tracker used for resources.
    Input::Tracker tracker;

    // ShaderManager used to create shaders.
    ion::gfxutils::ShaderManagerPtr shader_manager;

    // Handy Input::Reader instance.
    Input::Reader reader;

    // Creates a TempFile containing the given input, tries to read a
    // Graph::Scene from it, and returns the Scene after removing the file.
    Graph::ScenePtr ReadScene(const std::string &input) {
        TempFile file(input);
        return reader.ReadScene(file.GetPathString());
    }

    // Calls ReadScene(), then prints the resulting Ion graph to a string,
    // comparing with the expected string.
    bool ReadSceneAndCompare(const std::string &input,
                             const std::string &expected_output) {
        TempFile file(input);
        Graph::ScenePtr scene = reader.ReadScene(file.GetPathString());
        EXPECT_NOT_NULL(scene.get());

        std::ostringstream out;
        ion::gfxutils::Printer printer;
        printer.EnableAddressPrinting(false);
        printer.EnableFullShapePrinting(true);
        printer.SetFloatCleanTolerance(1e-5f);  // Clean values close to zero.
        printer.PrintScene(scene->GetRootNode()->GetIonNode(), out);

        size_t index;
        if (! Util::CompareStrings(out.str(), expected_output, index)) {
            std::cerr << "*** Strings differ at index " << index << "\n";
            std::cerr << "*** Expected:\n" << expected_output << "\n";
            std::cerr << "*** Actual:\n"   << out.str() << "\n";
            return false;
        }
        return true;
    }
};

TEST_F(InputTest, EmptyScene) {
    Graph::ScenePtr scene = ReadScene("Scene \"MyScene\" {}\n");
    EXPECT_NOT_NULL(scene.get());
    EXPECT_EQ("MyScene", scene->GetName());
    EXPECT_NULL(scene->GetRootNode());
}

TEST_F(InputTest, RootNode) {
    std::string input =
        "Scene \"MyScene\" {\n"
        "  root: Node \"MyNode\" {}\n"
        "}\n";
    std::string expected =
        "ION Node \"MyNode\" {\n"
        "  Enabled: true\n"
        "  ION Uniform {\n"
        "    Name: \"uModelviewMatrix\"\n"
        "    Type: Matrix4x4\n"
        "    Value: [[1, 0, 0, 0]\n"
        "            [0, 1, 0, 0]\n"
        "            [0, 0, 1, 0]\n"
        "            [0, 0, 0, 1]]\n"
        "  }\n"
        "}\n";
    EXPECT_TRUE(ReadSceneAndCompare(input, expected));
}

TEST_F(InputTest, Transform) {
    std::string input =
        "Scene {\n"
        "  root: Node {\n"
        "    scale:       2 3 4,\n"
        "    rotation:    0 1 0 -90,\n"
        "    translation: 100 200 300,\n"
        "  }\n"
        "}\n";
    std::string expected =
        "ION Node {\n"
        "  Enabled: true\n"
        "  ION Uniform {\n"
        "    Name: \"uModelviewMatrix\"\n"
        "    Type: Matrix4x4\n"
        "    Value: [[0, 0, -4, 100]\n"
        "            [0, 3, 0, 200]\n"
        "            [2, 0, 0, 300]\n"
        "            [0, 0, 0, 1]]\n"
        "  }\n"
        "}\n";
    EXPECT_TRUE(ReadSceneAndCompare(input, expected));
}

TEST_F(InputTest, OneChild) {
    std::string input =
        "Scene \"MyScene\" {\n"
        "  root: Node {\n"
        "    children: [\n"
        "      Node {}\n"
        "    ]\n"
        "  }\n"
        "}\n";
    std::string expected =
        "ION Node {\n"
        "  Enabled: true\n"
        "  ION Uniform {\n"
        "    Name: \"uModelviewMatrix\"\n"
        "    Type: Matrix4x4\n"
        "    Value: [[1, 0, 0, 0]\n"
        "            [0, 1, 0, 0]\n"
        "            [0, 0, 1, 0]\n"
        "            [0, 0, 0, 1]]\n"
        "  }\n"
        "  ION Node {\n"
        "    Enabled: true\n"
        "    ION Uniform {\n"
        "      Name: \"uModelviewMatrix\"\n"
        "      Type: Matrix4x4\n"
        "      Value: [[1, 0, 0, 0]\n"
        "              [0, 1, 0, 0]\n"
        "              [0, 0, 1, 0]\n"
        "              [0, 0, 0, 1]]\n"
        "    }\n"
        "  }\n"
        "}\n";
    EXPECT_TRUE(ReadSceneAndCompare(input, expected));
}

TEST_F(InputTest, TwoChildrenAndNames) {
    std::string input =
        "Scene \"MyScene\" {\n"
        "  root: Node \"Parent\" {\n"
        "    children: [\n"
        "      Node \"AChild\" {},\n"
        "      Node \"AnotherChild\" {},\n"
        "    ]\n"
        "  }\n"
        "}\n";
    std::string expected =
        "ION Node \"Parent\" {\n"
        "  Enabled: true\n"
        "  ION Uniform {\n"
        "    Name: \"uModelviewMatrix\"\n"
        "    Type: Matrix4x4\n"
        "    Value: [[1, 0, 0, 0]\n"
        "            [0, 1, 0, 0]\n"
        "            [0, 0, 1, 0]\n"
        "            [0, 0, 0, 1]]\n"
        "  }\n"
        "  ION Node \"AChild\" {\n"
        "    Enabled: true\n"
        "    ION Uniform {\n"
        "      Name: \"uModelviewMatrix\"\n"
        "      Type: Matrix4x4\n"
        "      Value: [[1, 0, 0, 0]\n"
        "              [0, 1, 0, 0]\n"
        "              [0, 0, 1, 0]\n"
        "              [0, 0, 0, 1]]\n"
        "    }\n"
        "  }\n"
        "  ION Node \"AnotherChild\" {\n"
        "    Enabled: true\n"
        "    ION Uniform {\n"
        "      Name: \"uModelviewMatrix\"\n"
        "      Type: Matrix4x4\n"
        "      Value: [[1, 0, 0, 0]\n"
        "              [0, 1, 0, 0]\n"
        "              [0, 0, 1, 0]\n"
        "              [0, 0, 0, 1]]\n"
        "    }\n"
        "  }\n"
        "}\n";
    EXPECT_TRUE(ReadSceneAndCompare(input, expected));
}

TEST_F(InputTest, Instances) {
    std::string input =
        "Scene {\n"
        "  root: Node \"A\" {\n"
        "    children: [\n"
        "      Node \"B1\" {\n"
        "        children: [ Node \"C\" {} ],\n"
        "      },\n"
        "      Node \"B2\" {\n"
        "        children: [ Node \"C\"; ],\n"  // Instanced
        "      },\n"
        "    ]\n"
        "  }\n"
        "}\n";
    Graph::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene.get());
    Graph::NodePtr root = scene->GetRootNode();
    EXPECT_NOT_NULL(root);
    EXPECT_EQ("A", root->GetName());
    EXPECT_EQ(2U, root->GetChildren().size());
    Graph::NodePtr b1 = root->GetChildren()[0];
    Graph::NodePtr b2 = root->GetChildren()[1];
    EXPECT_EQ("B1", b1->GetName());
    EXPECT_EQ("B2", b2->GetName());
    EXPECT_EQ(1U, b1->GetChildren().size());
    EXPECT_EQ(1U, b2->GetChildren().size());

    // Check that the instanced nodes are identical.
    Graph::NodePtr b1c = b1->GetChildren()[0];
    Graph::NodePtr b2c = b2->GetChildren()[0];
    EXPECT_EQ(b1c.get(), b2c.get());
}

TEST_F(InputTest, Enabled) {
    std::string input = "Scene { root: Node { enabled: false } }\n";
    std::string expected =
        "ION Node {\n"
        "  Enabled: false\n"
        "  ION Uniform {\n"
        "    Name: \"uModelviewMatrix\"\n"
        "    Type: Matrix4x4\n"
        "    Value: [[1, 0, 0, 0]\n"
        "            [0, 1, 0, 0]\n"
        "            [0, 0, 1, 0]\n"
        "            [0, 0, 0, 1]]\n"
        "  }\n"
        "}\n";
    EXPECT_TRUE(ReadSceneAndCompare(input, expected));
}

TEST_F(InputTest, Box) {
    std::string input =
        "Scene {\n"
        "  root: Node {\n"
        "    shapes: [\n"
        "      Box \"Box1\"  {\n"
        "        size: 1 2 3,\n"
        "      }\n"
        "    ]\n"
        "  }\n"
        "}\n";
    std::string expected =
        "ION Node {\n"
        "  Enabled: true\n"
        "  ION Uniform {\n"
        "    Name: \"uModelviewMatrix\"\n"
        "    Type: Matrix4x4\n"
        "    Value: [[1, 0, 0, 0]\n"
        "            [0, 1, 0, 0]\n"
        "            [0, 0, 1, 0]\n"
        "            [0, 0, 0, 1]]\n"
        "  }\n"
        "  ION Shape \"Box1\" {\n"
        "    Primitive Type: Triangles\n"
        "    ION AttributeArray {\n"
        "      Buffer Values: {\n"
        "        v 0: [-0.5, -1, 1.5], [0, 0], [0, 0, 1]\n"
        "        v 1: [0.5, -1, 1.5], [1, 0], [0, 0, 1]\n"
        "        v 2: [0.5, 1, 1.5], [1, 1], [0, 0, 1]\n"
        "        v 3: [-0.5, 1, 1.5], [0, 1], [0, 0, 1]\n"
        "        v 4: [0.5, -1, -1.5], [0, 0], [0, 0, -1]\n"
        "        v 5: [-0.5, -1, -1.5], [1, 0], [0, 0, -1]\n"
        "        v 6: [-0.5, 1, -1.5], [1, 1], [0, 0, -1]\n"
        "        v 7: [0.5, 1, -1.5], [0, 1], [0, 0, -1]\n"
        "        v 8: [0.5, -1, 1.5], [0, 0], [1, 0, 0]\n"
        "        v 9: [0.5, -1, -1.5], [1, 0], [1, 0, 0]\n"
        "        v 10: [0.5, 1, -1.5], [1, 1], [1, 0, 0]\n"
        "        v 11: [0.5, 1, 1.5], [0, 1], [1, 0, 0]\n"
        "        v 12: [-0.5, -1, -1.5], [0, 0], [-1, 0, 0]\n"
        "        v 13: [-0.5, -1, 1.5], [1, 0], [-1, 0, 0]\n"
        "        v 14: [-0.5, 1, 1.5], [1, 1], [-1, 0, 0]\n"
        "        v 15: [-0.5, 1, -1.5], [0, 1], [-1, 0, 0]\n"
        "        v 16: [-0.5, 1, 1.5], [0, 0], [0, 1, 0]\n"
        "        v 17: [0.5, 1, 1.5], [1, 0], [0, 1, 0]\n"
        "        v 18: [0.5, 1, -1.5], [1, 1], [0, 1, 0]\n"
        "        v 19: [-0.5, 1, -1.5], [0, 1], [0, 1, 0]\n"
        "        v 20: [-0.5, -1, -1.5], [0, 0], [0, -1, 0]\n"
        "        v 21: [0.5, -1, -1.5], [1, 0], [0, -1, 0]\n"
        "        v 22: [0.5, -1, 1.5], [1, 1], [0, -1, 0]\n"
        "        v 23: [-0.5, -1, 1.5], [0, 1], [0, -1, 0]\n"
        "      }\n"
        "      ION Attribute (Buffer) {\n"
        "        Name: \"aVertex\"\n"
        "        Enabled: true\n"
        "        Normalized: false\n"
        "      }\n"
        "      ION Attribute (Buffer) {\n"
        "        Name: \"aTexCoords\"\n"
        "        Enabled: true\n"
        "        Normalized: false\n"
        "      }\n"
        "      ION Attribute (Buffer) {\n"
        "        Name: \"aNormal\"\n"
        "        Enabled: true\n"
        "        Normalized: false\n"
        "      }\n"
        "    }\n"
        "    ION IndexBuffer {\n"
        "      Type: Unsigned Short\n"
        "      Target: Elementbuffer\n"
        "      Indices: [0 - 9: 0, 1, 2, 0, 2, 3, 4, 5, 6, 4,\n"
        "                10 - 19: 6, 7, 8, 9, 10, 8, 10, 11, 12, 13,\n"
        "                20 - 29: 14, 12, 14, 15, 16, 17, 18, 16, 18, 19,\n"
        "                30 - 35: 20, 21, 22, 20, 22, 23]\n"
        "    }\n"
        "  }\n"
        "}\n";
    EXPECT_TRUE(ReadSceneAndCompare(input, expected));
}

TEST_F(InputTest, Cylinder) {
    std::string input =
        "Scene {\n"
        "  root:Node {\n"
        "    shapes: [\n"
        "      Cylinder \"Cyl1\" {\n"
        "        top_radius:       2,\n"
        "        bottom_radius:    3,\n"
        "        height:           10,\n"
        "        has_top_cap:      T,\n"
        "        has_bottom_cap:   F,\n"
        "        shaft_band_count: 2,\n"
        "        cap_band_count:   2,\n"
        "        sector_count:     4,\n"
        "      }\n"
        "    ]\n"
        "  }\n"
        "}\n";
    std::string expected =
        "ION Node {\n"
        "  Enabled: true\n"
        "  ION Uniform {\n"
        "    Name: \"uModelviewMatrix\"\n"
        "    Type: Matrix4x4\n"
        "    Value: [[1, 0, 0, 0]\n"
        "            [0, 1, 0, 0]\n"
        "            [0, 0, 1, 0]\n"
        "            [0, 0, 0, 1]]\n"
        "  }\n"
        "  ION Shape \"Cyl1\" {\n"
        "    Primitive Type: Triangles\n"
        "    ION AttributeArray {\n"
        "      Buffer Values: {\n"
        "        v 0: [0, 5, -2], [0, 1], [0, 0.0995037, -0.995037]\n"
        "        v 1: [-2, 5, 0], [0.25, 1], [-0.995037, 0.0995037, 0]\n"
        "        v 2: [0, 5, 2], [0.5, 1], [0, 0.0995037, 0.995037]\n"
        "        v 3: [2, 5, 0], [0.75, 1], [0.995037, 0.0995037, 0]\n"
        "        v 4: [0, 5, -2], [1, 1], [0, 0.0995037, -0.995037]\n"
        "        v 5: [0, 0, -2.5], [0, 0.5], [0, 0.0995037, -0.995037]\n"
        "        v 6: [-2.5, 0, 0], [0.25, 0.5], [-0.995037, 0.0995037, 0]\n"
        "        v 7: [0, 0, 2.5], [0.5, 0.5], [0, 0.0995037, 0.995037]\n"
        "        v 8: [2.5, 0, 0], [0.75, 0.5], [0.995037, 0.0995037, 0]\n"
        "        v 9: [0, 0, -2.5], [1, 0.5], [0, 0.0995037, -0.995037]\n"
        "        v 10: [0, -5, -3], [0, 0], [0, 0.0995037, -0.995037]\n"
        "        v 11: [-3, -5, 0], [0.25, 0], [-0.995037, 0.0995037, 0]\n"
        "        v 12: [0, -5, 3], [0.5, 0], [0, 0.0995037, 0.995037]\n"
        "        v 13: [3, -5, 0], [0.75, 0], [0.995037, 0.0995037, 0]\n"
        "        v 14: [0, -5, -3], [1, 0], [0, 0.0995037, -0.995037]\n"
        "        v 15: [0, 5, 0], [0.5, 0.5], [0, 1, 0]\n"
        "        v 16: [0, 5, -1], [0.5, 0.75], [0, 1, 0]\n"
        "        v 17: [-1, 5, 0], [0.25, 0.5], [0, 1, 0]\n"
        "        v 18: [0, 5, 1], [0.5, 0.25], [0, 1, 0]\n"
        "        v 19: [1, 5, 0], [0.75, 0.5], [0, 1, 0]\n"
        "        v 20: [0, 5, -1], [0.5, 0.75], [0, 1, 0]\n"
        "        v 21: [0, 5, -2], [0.5, 1], [0, 1, 0]\n"
        "        v 22: [-2, 5, 0], [0, 0.5], [0, 1, 0]\n"
        "        v 23: [0, 5, 2], [0.5, 0], [0, 1, 0]\n"
        "        v 24: [2, 5, 0], [1, 0.5], [0, 1, 0]\n"
        "        v 25: [0, 5, -2], [0.5, 1], [0, 1, 0]\n"
        "      }\n"
        "      ION Attribute (Buffer) {\n"
        "        Name: \"aVertex\"\n"
        "        Enabled: true\n"
        "        Normalized: false\n"
        "      }\n"
        "      ION Attribute (Buffer) {\n"
        "        Name: \"aTexCoords\"\n"
        "        Enabled: true\n"
        "        Normalized: false\n"
        "      }\n"
        "      ION Attribute (Buffer) {\n"
        "        Name: \"aNormal\"\n"
        "        Enabled: true\n"
        "        Normalized: false\n"
        "      }\n"
        "    }\n"
        "    ION IndexBuffer {\n"
        "      Type: Unsigned Short\n"
        "      Target: Elementbuffer\n"
        "      Indices: [0 - 9: 0, 5, 1, 1, 5, 6, 1, 6, 2, 2,\n"
        "                10 - 19: 6, 7, 2, 7, 3, 3, 7, 8, 3, 8,\n"
        "                20 - 29: 4, 4, 8, 9, 5, 10, 6, 6, 10, 11,\n"
        "                30 - 39: 6, 11, 7, 7, 11, 12, 7, 12, 8, 8,\n"
        "                40 - 49: 12, 13, 8, 13, 9, 9, 13, 14, 15, 16,\n"
        "                50 - 59: 17, 15, 17, 18, 15, 18, 19, 15, 19, 20,\n"
        "                60 - 69: 16, 21, 17, 17, 21, 22, 17, 22, 18, 18,\n"
        "                70 - 79: 22, 23, 18, 23, 19, 19, 23, 24, 19, 24,\n"
        "                80 - 83: 20, 20, 24, 25]\n"
        "    }\n"
        "  }\n"
        "}\n";
    EXPECT_TRUE(ReadSceneAndCompare(input, expected));
}
