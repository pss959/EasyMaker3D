#include <sstream>
#include <string>

#include <sstream>

#include "Loader.h"
#include "Testing.h"

#include <ion/gfx/node.h>
#include <ion/gfxutils/printer.h>

using ion::gfx::NodePtr;

// Tests that a Loader::Exception is thrown and that its message contains
// the given string pattern.
#define TEST_THROW_(STMT, PATTERN) \
    TEST_THROW(STMT, Loader::Exception, PATTERN)

class LoaderTest : public TestBase {
 protected:
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
        "Node {\n"
        "  name: \"Parent\",\n"
        "  children: [\n"
        "    Node { name:\"AChild\" },\n"
        "    Node { name:\"AnotherChild\" },\n"
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
