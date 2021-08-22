#include <sstream>
#include <string>

#include <ion/gfxutils/printer.h>

#include "Parser/Exception.h"
#include "SG/Node.h"
#include "SG/Typedefs.h"
#include "SG/Writer.h"
#include "SceneTestBase.h"
#include "Util/String.h"

// Tests that a Parser::Exception is thrown and that its message contains the
// given string pattern.
#define TEST_THROW_(STMT, PATTERN) TEST_THROW(STMT, Parser::Exception, PATTERN)

class ReaderTest : public SceneTestBase {
 protected:
    // Calls ReadScene(), then prints the resulting SG to a string, comparing
    // with the expected output string.
    bool ReadSceneAndCompare(const std::string &input,
                             const std::string &expected) {
        set_up_ion = false;
        SG::ScenePtr scene = ReadScene(input);
        EXPECT_NOT_NULL(scene.get());

        std::ostringstream out;
        SG::Writer writer;
        writer.WriteScene(*scene, out);
        return CompareResults(expected, out.str());
    }

    // Calls ReadScene(), then prints the resulting Ion graph to a string,
    // comparing with the expected string.
    bool ReadSceneAndCompareIon(const std::string &input,
                                const std::string &expected) {
        set_up_ion = true;
        SG::ScenePtr scene = ReadScene(input);
        EXPECT_NOT_NULL(scene.get());

        std::ostringstream out;
        ion::gfxutils::Printer printer;
        printer.EnableAddressPrinting(false);
        printer.EnableFullShapePrinting(true);
        printer.SetFloatCleanTolerance(1e-5f);  // Clean values close to zero.
        printer.PrintScene(scene->GetRootNode()->GetIonNode(), out);
        return CompareResults(expected, out.str());
    }

    //! Compares result strings and reports any differences.
    bool CompareResults(const std::string &expected,
                        const std::string &actual) {
        size_t index;
        if (! Util::CompareStrings(actual, expected, index)) {
            if (index < actual.size() && index < expected.size()) {
                EXPECT_NE(actual[index], expected[index]);
            }
            std::cerr << "*** Strings differ at index " << index << "\n";
            std::cerr << "*** Expected:\n" << expected << "\n";
            std::cerr << "*** Actual:\n"   << actual << "\n";
            std::cerr << "***   (";
            if (index < expected.size())
                std::cerr << "'" << expected[index] << "'";
            else
                std::cerr << "EOF";
            std::cerr << ") vs. (";
            if (index < actual.size())
                std::cerr << "'" << actual[index] << "'";
            else
                std::cerr << "EOF";
            std::cerr << ")\n";
            /* Uncomment for extra help.
            std::cerr << "*** 1-line Expected:" <<
                Util::ReplaceString(expected, "\n", "|") << "\n";
            std::cerr << "*** 1-line Actual:  " <<
                Util::ReplaceString(actual,   "\n", "|") << "\n";
            */
            return false;
        }
        return true;
    }
};

TEST_F(ReaderTest, EmptyScene) {
    SG::ScenePtr scene = ReadScene("Scene \"MyScene\" {}\n");
    EXPECT_NOT_NULL(scene.get());
    EXPECT_EQ("MyScene", scene->GetName());
    EXPECT_NULL(scene->GetCamera());
    EXPECT_NULL(scene->GetRootNode());
}

TEST_F(ReaderTest, RootNode) {
    std::string input =
        "Scene \"MyScene\" {\n"
        "  root: Node \"MyNode\" {}\n"
        "}\n";
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NULL(scene->GetCamera());
    EXPECT_NOT_NULL(scene->GetRootNode());
    EXPECT_EQ("MyNode", scene->GetRootNode()->GetName());
    EXPECT_EQ(0U, scene->GetRootNode()->GetChildren().size());
}

TEST_F(ReaderTest, AllTypes) {
    std::string input = ReadDataFile("AllTypes.mvn");
    EXPECT_TRUE(ReadSceneAndCompare(input, input));
}

TEST_F(ReaderTest, Instances) {
    std::string input = ReadDataFile("Instances.mvn");
    EXPECT_TRUE(ReadSceneAndCompare(input, input));
}

TEST_F(ReaderTest, SetUpIonRootNode) {
    std::string input =
        "Scene \"MyScene\" {\n"
        "  root: Node \"MyNode\" {}\n"
        "}\n";
    std::string expected =
        "ION Node \"MyNode\" {\n"
        "  Enabled: true\n"
        "}\n";
    EXPECT_TRUE(ReadSceneAndCompareIon(input, expected));
}

TEST_F(ReaderTest, Transform) {
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
    EXPECT_TRUE(ReadSceneAndCompareIon(input, expected));
}

TEST_F(ReaderTest, OneChild) {
    std::string input =
        "Scene \"MyScene\" {\n"
        "  root: Node {\n"
        "    children: [\n"
        "      Node \"ChildX\" {}\n"
        "    ]\n"
        "  }\n"
        "}\n";
    std::string expected =
        "ION Node {\n"
        "  Enabled: true\n"
        "  ION Node \"ChildX\" {\n"
        "    Enabled: true\n"
        "  }\n"
        "}\n";
    EXPECT_TRUE(ReadSceneAndCompareIon(input, expected));
}

TEST_F(ReaderTest, TwoChildrenAndNames) {
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
        "  ION Node \"AChild\" {\n"
        "    Enabled: true\n"
        "  }\n"
        "  ION Node \"AnotherChild\" {\n"
        "    Enabled: true\n"
        "  }\n"
        "}\n";
    EXPECT_TRUE(ReadSceneAndCompareIon(input, expected));
}

TEST_F(ReaderTest, Enabled) {
    std::string input = "Scene { root: Node { enabled: False } }\n";
    std::string expected =
        "ION Node {\n"
        "  Enabled: false\n"
        "}\n";
    EXPECT_TRUE(ReadSceneAndCompareIon(input, expected));
}

TEST_F(ReaderTest, Box) {
    std::string input =
        "Scene {\n"
        "  root: Node {\n"
        "    shapes: [\n"
        "      Box \"Box1\" {\n"
        "        size: 1 2 3,\n"
        "      }\n"
        "    ]\n"
        "  }\n"
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
    EXPECT_TRUE(ReadSceneAndCompareIon(input, expected));
}

TEST_F(ReaderTest, Cylinder) {
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
    EXPECT_TRUE(ReadSceneAndCompareIon(input, expected));
}
