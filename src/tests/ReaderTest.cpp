#include <sstream>
#include <string>

#include <sstream>

#include "Parser/Exception.h"
#include "SG/Node.h"
#include "SG/Reader.h"
#include "SG/Scene.h"
#include "SG/Typedefs.h"
#include "SG/Tracker.h"
#include "Testing.h"
#include "Util/String.h"

#include "SG/Writer.h"  // XXXX

#include <ion/gfxutils/shadermanager.h>

// Tests that a Parser::Exception is thrown and that its message contains the
// given string pattern.
#define TEST_THROW_(STMT, PATTERN) TEST_THROW(STMT, Parser::Exception, PATTERN)

class ReaderTest : public TestBase {
 protected:
    ReaderTest() :
        shader_manager(new ion::gfxutils::ShaderManager),
        reader(tracker, *shader_manager) {}

    // ShaderManager used to create shaders.
    ion::gfxutils::ShaderManagerPtr shader_manager;

    // Tracker used for resources.
    SG::Tracker tracker;

    // Handy SG::Reader instance.
    SG::Reader reader;

    // Creates a TempFile containing the given input, tries to read a Scene
    // from it, and returns the Scene after removing the file.
    SG::ScenePtr ReadScene(const std::string &input) {
        TempFile file(input);
        return reader.ReadScene(file.GetPathString());
    }

    // Calls ReadScene(), then prints the resulting SG to a string, comparing
    // with the expected string.
    bool ReadSceneAndCompare(const std::string &input,
                             const std::string &expected_output) {
        SG::ScenePtr scene = ReadScene(input);
        EXPECT_NOT_NULL(scene.get());

        std::ostringstream out;
        SG::Writer writer;
        writer.WriteScene(*scene, out);
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

TEST_F(ReaderTest, EmptyScene) {
    SG::ScenePtr scene = ReadScene("Scene \"MyScene\" {}\n");
    EXPECT_NOT_NULL(scene.get());
    EXPECT_EQ("MyScene", scene->GetName());
    // XXXX This requires Finalize():
    // XXXX EXPECT_NOT_NULL(scene->GetRootNode());
    // XXXX EXPECT_EQ(0U, scene->GetRootNode()->GetChildren().size());
}

TEST_F(ReaderTest, RootNode) {
    std::string input =
        "Scene \"MyScene\" {\n"
        "  root: Node \"MyNode\" {}\n"
        "}\n";
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene->GetRootNode());
    EXPECT_EQ("MyNode", scene->GetRootNode()->GetName());
    EXPECT_EQ(0U, scene->GetRootNode()->GetChildren().size());
}

TEST_F(ReaderTest, XXXX) {
    std::string input =
        "Scene \"MyScene\" {\n"
        "  camera: Camera {\n"
        "    near: 1234,\n"
        "  },\n"
        "  root: Node \"MyNode\" {\n"
        "    scale: 1 2 3,\n"
        "    rotation: 0 1 0 45,\n"
        "    translation: -7 -8 -9,\n"
        "    state_table: StateTable {\n"
        "      clear_color: .4 .4 .4 1,\n"
        "      depth_test_enabled: true,\n"
        "      cull_face_enabled: true,\n"
        "    }\n"
        "  },\n"
        "}\n";
    std::string expected =
        "Scene \"MyScene\" {\n"
        "  camera: Camera {\n"
        "    near: 1234,\n"
        "  }\n"
        "  root: Node \"MyNode\" {\n"
        "    scale: V[1, 2, 3],\n"
        "    rotation: ROT[V[0, 1, 0]: 45 deg],\n"
        "    translation: V[-7, -8, -9],\n"
        "    state_table: StateTable {\n"
        "      clear_color: V[0.4, 0.4, 0.4, 1],\n"
        "      depth_test_enabled: true,\n"
        "      cull_face_enabled: true,\n"
        "    }\n"
        "  }\n"
        "}\n";

    ReadSceneAndCompare(input, expected);
}
