#include <sstream>
#include <string>

#include <sstream>

#include "Loader.h"
#include "Managers/ResourceManager.h"
#include "Testing.h"

#include <ion/gfx/node.h>
#include <ion/gfxutils/printer.h>
#include <ion/math/matrix.h>
#include <ion/math/matrixutils.h>

using ion::gfx::NodePtr;
using ion::math::Matrix4f;

// Tests that a Loader::Exception is thrown and that its message contains
// the given string pattern.
#define TEST_THROW_(STMT, PATTERN) \
    TEST_THROW(STMT, Loader::Exception, PATTERN)

class LoaderTest : public TestBase {
 protected:
    LoaderTest() : loader(resource_manager) {}

    // ResourceManager used for Loader.
    ResourceManager resource_manager;

    // Handy Loader instance.
    Loader loader;

    // Creates a TempFile containing the given input, tries to load a Node from
    // it, and returns the Node after removing the file.
    NodePtr LoadNode(const std::string &input) {
        TempFile file(input);
        return loader.LoadNode(file.GetPathString());
    }

    // Calls LoadNode(), then prints the resulting Ion graph to a string,
    // comparing with the expected string.
    void LoadNodeAndCompare(const std::string &input,
                            const std::string &expected_output) {
        TempFile file(input);
        NodePtr node = loader.LoadNode(file.GetPathString());
        EXPECT_NOT_NULL(node.Get());

        std::ostringstream out;
        ion::gfxutils::Printer printer;
        printer.EnableAddressPrinting(false);
        printer.EnableFullShapePrinting(true);
        printer.SetFloatCleanTolerance(1e-5f);  // Clean values close to zero.
        printer.PrintScene(node, out);
        EXPECT_EQ(expected_output, out.str());
    }
};

TEST_F(LoaderTest, EmptyNode) {
    NodePtr node = LoadNode("Node {}\n");
    EXPECT_NOT_NULL(node.Get());
    EXPECT_EQ(0U, node->GetChildren().size());
    EXPECT_EQ(0U, node->GetShapes().size());
}

TEST_F(LoaderTest, OneChild) {
    std::string input =
        "Node {\n"
        "  children: [\n"
        "    Node {}\n"
        "  ]\n"
        "}\n";
    std::string expected =
        "ION Node {\n"
        "  Enabled: true\n"
        "  ION Node {\n"
        "    Enabled: true\n"
        "  }\n"
        "}\n";
    LoadNodeAndCompare(input, expected);
}

TEST_F(LoaderTest, TwoChildrenAndNames) {
    std::string input =
        "Node \"Parent\" {\n"
        "  children: [\n"
        "    Node \"AChild\" {},\n"
        "    Node \"AnotherChild\" {},\n"
        "  ]\n"
        "}\n";
    std::string expected =
        "ION Node \"Parent\" {\n"
        "  Enabled: true\n"
        "  ION Node \"AChild\" {\n"
        "    Enabled: true\n"
        "  }\n"
        "  ION Node \"AnotherChild\" {\n"
        "    Enabled: true\n"
        "  }\n"
        "}\n";
    LoadNodeAndCompare(input, expected);
}

TEST_F(LoaderTest, Enabled) {
    std::string input = "Node { enabled: false }\n";
    std::string expected =
        "ION Node {\n"
        "  Enabled: false\n"
        "}\n";
    LoadNodeAndCompare(input, expected);
}

TEST_F(LoaderTest, Transform) {
    std::string input =
        "Node {\n"
        "  scale:       1 2 3,\n"
        "  rotation:    0 1 0 90,\n"
        "  translation: 100 200 300,\n"
        "}\n";

    const Matrix4f expected(0.f,  0.f, 3.f, 100.f,
                            0.f,  2.f, 0.f, 200.f,
                            -1.f, 0.f, 0.f, 300.f,
                            0.f,  0.f, 0.f, 1.f);
    NodePtr node = LoadNode(input);
    const auto &uniforms = node->GetUniforms();
    EXPECT_EQ(1U, uniforms.size());
    const auto &umv = uniforms[0];
    const auto &spec = *umv.GetRegistry().GetSpec(umv);
    EXPECT_EQ("uModelviewMatrix", spec.name);
    const Matrix4f &actual = umv.GetValue<Matrix4f>();
    EXPECT_PRED2([](const Matrix4f &expected, const Matrix4f &actual) {
        return ion::math::MatricesAlmostEqual(expected, actual, 1e-5f);
    }, expected, actual);
}

TEST_F(LoaderTest, Box) {
    std::string input =
        "Node {\n"
        "  shapes: [\n"
        "    Box \"Box1\" {\n"
        "      size: 1 2 3,\n"
        "    }\n"
        "  ]\n"
        "}\n";
    std::string expected =
        "ION Node {\n"
        "  Enabled: true\n"
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
    LoadNodeAndCompare(input, expected);
}

TEST_F(LoaderTest, Cylinder) {
    std::string input =
        "Node {\n"
        "  shapes: [\n"
        "    Cylinder \"Cyl1\" {\n"
        "      top_radius:       2,\n"
        "      bottom_radius:    3,\n"
        "      height:           10,\n"
        "      has_top_cap:      T,\n"
        "      has_bottom_cap:   F,\n"
        "      shaft_band_count: 2,\n"
        "      cap_band_count:   2,\n"
        "      sector_count:     4,\n"
        "    }\n"
        "  ]\n"
        "}\n";
    std::string expected =
        "ION Node {\n"
        "  Enabled: true\n"
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
    LoadNodeAndCompare(input, expected);
}
