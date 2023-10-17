#include <sstream>
#include <string>

#include <ion/gfxutils/printer.h>

#include "Parser/Exception.h"
#include "Parser/Parser.h"
#include "Parser/Writer.h"
#include "SG/Node.h"
#include "SG/Reader.h"
#include "Tests/SceneTestBase.h"
#include "Tests/TempFile.h"
#include "Tests/Testing2.h"
#include "Util/String.h"

// Tests that a Parser::Exception is thrown and that its message contains the
// given string pattern.
#define TEST_THROW_(STMT, PATTERN) TEST_THROW(STMT, Parser::Exception, PATTERN)

/// \ingroup Tests
class ReaderTest : public SceneTestBase {
 protected:
    /// Parses the given input string, then writes the resulting data to a
    /// string, comparing with the expected output string.
    bool ParseStringAndCompare(const Str &input, const Str &expected) {
        Parser::Parser parser;
        Parser::ObjectPtr root = parser.ParseFromString(input);

        std::ostringstream out;
        Parser::Writer writer(out);
        writer.WriteObject(*root);
        return CompareStrings(FixString(expected), FixString(out.str()));
    }

    /// Calls ReadScene(), then prints the resulting SG to a string, comparing
    /// with the expected output string.
    bool ReadSceneAndCompare(const Str &input, const Str &expected) {
        SG::ScenePtr scene = ReadScene(input, false);
        EXPECT_NOT_NULL(scene.get());

        std::ostringstream out;
        Parser::Writer writer(out);
        writer.WriteObject(*scene);
        return CompareStrings(FixString(expected), FixString(out.str()));
    }

    /// Calls BuildAndReadScene(), then prints the resulting Ion graph to a
    /// string, comparing with the expected string.
    bool ReadSceneAndCompareIon(const Str &contents, const Str &expected) {
        SG::ScenePtr scene = BuildAndReadScene(contents);
        EXPECT_NOT_NULL(scene.get());

        std::ostringstream out;
        ion::gfxutils::Printer printer;
        printer.EnableAddressPrinting(false);
        printer.EnableFullShapePrinting(true);
        printer.SetFloatCleanTolerance(1e-5f);  // Clean values close to zero.
        printer.PrintScene(scene->GetRootNode()->GetIonNode(), out);
        return CompareStrings(FixString(expected), FixString(out.str()));
    }
};

TEST_F(ReaderTest, GetAndSetPath) {
    TempFile file("Scene \"MyScene\" {}\n");
    SG::Reader reader;
    auto scene = reader.ReadScene(file.GetPath(),
                                  GetIonContext()->GetFileMap());
    EXPECT_NOT_NULL(scene.get());

    EXPECT_EQ(file.GetPath(), scene->GetPath());

    scene->SetPath("/some/other/path.emd");
    EXPECT_EQ("/some/other/path.emd", scene->GetPath());
}

TEST_F(ReaderTest, EmptyScene) {
    SG::ScenePtr scene = ReadScene("Scene \"MyScene\" {}\n");
    EXPECT_NOT_NULL(scene.get());
    EXPECT_EQ("MyScene", scene->GetName());
    EXPECT_NULL(scene->GetGantry());
    EXPECT_TRUE(scene->GetLights().empty());
    EXPECT_TRUE(scene->GetRenderPasses().empty());
    EXPECT_NULL(scene->GetRootNode());
}

TEST_F(ReaderTest, RootNode) {
    SG::ScenePtr scene = BuildAndReadScene("");
    EXPECT_NULL(scene->GetGantry());
    EXPECT_NOT_NULL(scene->GetRootNode());
    EXPECT_EQ("Root", scene->GetRootNode()->GetName());
    EXPECT_EQ(0U, scene->GetRootNode()->GetChildren().size());
}

TEST_F(ReaderTest, Settings) {
    const Str input = ReadDataFile("Settings.emd");
    EXPECT_TRUE(ParseStringAndCompare(input, input));
}

TEST_F(ReaderTest, AllTypes) {
    const Str input = ReadDataFile("AllTypes.emd");
    EXPECT_TRUE(ReadSceneAndCompare(input, input));

    // Create Ion data for this scene to test all shapes.
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene.get());
}

TEST_F(ReaderTest, Instances) {
    const Str input = ReadDataFile("Instances.emd");
    EXPECT_TRUE(ReadSceneAndCompare(input, input));
}

TEST_F(ReaderTest, SetUpIonRootNode) {
    const Str expected =
        "ION Node \"Root\" {\n"
        "  Enabled: true\n"
        "}\n";
    EXPECT_TRUE(ReadSceneAndCompareIon("", expected));
}

TEST_F(ReaderTest, IonTransform) {
    const Str contents = R"(
  scale:       2 3 4,
  rotation:    0 1 0 -90,
  translation: 100 200 300,
)";

    const Str expected =
R"(ION Node "Root" {
  Enabled: true
  ION UniformBlock {
    Enabled: true
    ION Uniform {
      Name: "uModelMatrix"
      Type: Matrix4x4
      Value: [[0, 0, -4, 100]
              [0, 3, 0, 200]
              [2, 0, 0, 300]
              [0, 0, 0, 1]]
    }
    ION Uniform {
      Name: "uModelviewMatrix"
      Type: Matrix4x4
      Value: [[0, 0, -4, 100]
              [0, 3, 0, 200]
              [2, 0, 0, 300]
              [0, 0, 0, 1]]
    }
  }
}
)";

    EXPECT_TRUE(ReadSceneAndCompareIon(contents, expected));
}

TEST_F(ReaderTest, OneChild) {
    const Str contents = R"(
  children: [
    Node "ChildX" {}
  ]
)";
    const Str expected =
R"(ION Node "Root" {
  Enabled: true
  ION Node "ChildX" {
    Enabled: true
  }
}
)";

    EXPECT_TRUE(ReadSceneAndCompareIon(contents, expected));
}

TEST_F(ReaderTest, TwoChildrenAndNames) {
    const Str contents = R"(
  children: [
    Node "AChild" {},
    Node "AnotherChild" {},
  ]
)";
    const Str expected =
R"(ION Node "Root" {
  Enabled: true
  ION Node "AChild" {
    Enabled: true
  }
  ION Node "AnotherChild" {
    Enabled: true
  }
}
)";
    EXPECT_TRUE(ReadSceneAndCompareIon(contents, expected));
}

TEST_F(ReaderTest, Box) {
    const Str contents = R"(
  shapes: [
    Box "Box1" {
      size: 1 2 3,
    }
  ]
)";
    const Str expected =
R"(ION Node "Root" {
  Enabled: true
  ION Shape "Box1" {
    Primitive Type: Triangles
    ION AttributeArray {
      Buffer Values: {
        v 0: [-0.5, -1, 1.5], [0, 0], [0, 0, 1]
        v 1: [0.5, -1, 1.5], [1, 0], [0, 0, 1]
        v 2: [0.5, 1, 1.5], [1, 1], [0, 0, 1]
        v 3: [-0.5, 1, 1.5], [0, 1], [0, 0, 1]
        v 4: [0.5, -1, -1.5], [0, 0], [0, 0, -1]
        v 5: [-0.5, -1, -1.5], [1, 0], [0, 0, -1]
        v 6: [-0.5, 1, -1.5], [1, 1], [0, 0, -1]
        v 7: [0.5, 1, -1.5], [0, 1], [0, 0, -1]
        v 8: [0.5, -1, 1.5], [0, 0], [1, 0, 0]
        v 9: [0.5, -1, -1.5], [1, 0], [1, 0, 0]
        v 10: [0.5, 1, -1.5], [1, 1], [1, 0, 0]
        v 11: [0.5, 1, 1.5], [0, 1], [1, 0, 0]
        v 12: [-0.5, -1, -1.5], [0, 0], [-1, 0, 0]
        v 13: [-0.5, -1, 1.5], [1, 0], [-1, 0, 0]
        v 14: [-0.5, 1, 1.5], [1, 1], [-1, 0, 0]
        v 15: [-0.5, 1, -1.5], [0, 1], [-1, 0, 0]
        v 16: [-0.5, 1, 1.5], [0, 0], [0, 1, 0]
        v 17: [0.5, 1, 1.5], [1, 0], [0, 1, 0]
        v 18: [0.5, 1, -1.5], [1, 1], [0, 1, 0]
        v 19: [-0.5, 1, -1.5], [0, 1], [0, 1, 0]
        v 20: [-0.5, -1, -1.5], [0, 0], [0, -1, 0]
        v 21: [0.5, -1, -1.5], [1, 0], [0, -1, 0]
        v 22: [0.5, -1, 1.5], [1, 1], [0, -1, 0]
        v 23: [-0.5, -1, 1.5], [0, 1], [0, -1, 0]
      }
      ION Attribute (Buffer) {
        Name: "aVertex"
        Enabled: true
        Normalized: false
      }
      ION Attribute (Buffer) {
        Name: "aTexCoords"
        Enabled: true
        Normalized: false
      }
      ION Attribute (Buffer) {
        Name: "aNormal"
        Enabled: true
        Normalized: false
      }
    }
    ION IndexBuffer {
      Type: Unsigned Short
      Target: Elementbuffer
      Indices: [0 - 9: 0, 1, 2, 0, 2, 3, 4, 5, 6, 4,
                10 - 19: 6, 7, 8, 9, 10, 8, 10, 11, 12, 13,
                20 - 29: 14, 12, 14, 15, 16, 17, 18, 16, 18, 19,
                30 - 35: 20, 21, 22, 20, 22, 23]
    }
  }
}
)";
    EXPECT_TRUE(ReadSceneAndCompareIon(contents, expected));
}

TEST_F(ReaderTest, Cylinder) {
    const Str contents = R"(
  shapes: [
    Cylinder "Cyl1" {
      top_radius:       2,
      bottom_radius:    3,
      height:           10,
      has_top_cap:      T,
      has_bottom_cap:   F,
      shaft_band_count: 2,
      cap_band_count:   2,
      sector_count:     4,
    }
  ]
)";
    const Str expected =
R"(ION Node "Root" {
  Enabled: true
  ION Shape "Cyl1" {
    Primitive Type: Triangles
    ION AttributeArray {
      Buffer Values: {
        v 0: [0, 5, -2], [0, 1], [0, 0.0995037, -0.995037]
        v 1: [-2, 5, 0], [0.25, 1], [-0.995037, 0.0995037, 0]
        v 2: [0, 5, 2], [0.5, 1], [0, 0.0995037, 0.995037]
        v 3: [2, 5, 0], [0.75, 1], [0.995037, 0.0995037, 0]
        v 4: [0, 5, -2], [1, 1], [0, 0.0995037, -0.995037]
        v 5: [0, 0, -2.5], [0, 0.5], [0, 0.0995037, -0.995037]
        v 6: [-2.5, 0, 0], [0.25, 0.5], [-0.995037, 0.0995037, 0]
        v 7: [0, 0, 2.5], [0.5, 0.5], [0, 0.0995037, 0.995037]
        v 8: [2.5, 0, 0], [0.75, 0.5], [0.995037, 0.0995037, 0]
        v 9: [0, 0, -2.5], [1, 0.5], [0, 0.0995037, -0.995037]
        v 10: [0, -5, -3], [0, 0], [0, 0.0995037, -0.995037]
        v 11: [-3, -5, 0], [0.25, 0], [-0.995037, 0.0995037, 0]
        v 12: [0, -5, 3], [0.5, 0], [0, 0.0995037, 0.995037]
        v 13: [3, -5, 0], [0.75, 0], [0.995037, 0.0995037, 0]
        v 14: [0, -5, -3], [1, 0], [0, 0.0995037, -0.995037]
        v 15: [0, 5, 0], [0.5, 0.5], [0, 1, 0]
        v 16: [0, 5, -1], [0.5, 0.75], [0, 1, 0]
        v 17: [-1, 5, 0], [0.25, 0.5], [0, 1, 0]
        v 18: [0, 5, 1], [0.5, 0.25], [0, 1, 0]
        v 19: [1, 5, 0], [0.75, 0.5], [0, 1, 0]
        v 20: [0, 5, -1], [0.5, 0.75], [0, 1, 0]
        v 21: [0, 5, -2], [0.5, 1], [0, 1, 0]
        v 22: [-2, 5, 0], [0, 0.5], [0, 1, 0]
        v 23: [0, 5, 2], [0.5, 0], [0, 1, 0]
        v 24: [2, 5, 0], [1, 0.5], [0, 1, 0]
        v 25: [0, 5, -2], [0.5, 1], [0, 1, 0]
      }
      ION Attribute (Buffer) {
        Name: "aVertex"
        Enabled: true
        Normalized: false
      }
      ION Attribute (Buffer) {
        Name: "aTexCoords"
        Enabled: true
        Normalized: false
      }
      ION Attribute (Buffer) {
        Name: "aNormal"
        Enabled: true
        Normalized: false
      }
    }
    ION IndexBuffer {
      Type: Unsigned Short
      Target: Elementbuffer
      Indices: [0 - 9: 0, 5, 1, 1, 5, 6, 1, 6, 2, 2,
                10 - 19: 6, 7, 2, 7, 3, 3, 7, 8, 3, 8,
                20 - 29: 4, 4, 8, 9, 5, 10, 6, 6, 10, 11,
                30 - 39: 6, 11, 7, 7, 11, 12, 7, 12, 8, 8,
                40 - 49: 12, 13, 8, 13, 9, 9, 13, 14, 15, 16,
                50 - 59: 17, 15, 17, 18, 15, 18, 19, 15, 19, 20,
                60 - 69: 16, 21, 17, 17, 21, 22, 17, 22, 18, 18,
                70 - 79: 22, 23, 18, 23, 19, 19, 23, 24, 19, 24,
                80 - 83: 20, 20, 24, 25]
    }
  }
}
)";
    EXPECT_TRUE(ReadSceneAndCompareIon(contents, expected));
}

TEST_F(ReaderTest, EmptyPass) {
    const Str input = R"(
Scene "MyScene" {
  render_passes: [
    LightingPass "Lighting" {}
  ]
}
)";

    TEST_THROW_(ReadScene(input), "No shader programs");
}

