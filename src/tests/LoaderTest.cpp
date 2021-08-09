#include <sstream>
#include <string>

#include "Loader.h"
#include "Testing.h"

#include <ion/gfx/node.h>

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
};

TEST_F(LoaderTest, EmptyNode) {
    NodePtr node = LoadNode("Node {}\n");
    EXPECT_NOT_NULL(node.Get());
    EXPECT_EQ(0U, node->GetChildren().size());
    EXPECT_EQ(0U, node->GetShapes().size());
}
