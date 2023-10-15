#include <algorithm>
#include <set>
#include <typeinfo>

#include "Tools/FindTools.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class FindToolsTest : public SceneTestBase {};

TEST_F(FindToolsTest, FindTools) {
    // Need to set up the shelves with tool icons.
    const Str contents = R"(
  children: [
    <"nodes/DimensionColors.emd">,
    <"nodes/Tools.emd">,
    <"nodes/Shelf.emd">,
    <"nodes/Shelves.emd">,
  ]
)";
    auto scene = ReadRealScene(contents);

    const size_t kExpectedToolCount = 20;

    // Test that the number of Tool types is the expected amount.
    const auto tools = FindTools(*scene);
    EXPECT_EQ(kExpectedToolCount, tools.size());

    // Test that there are no duplicate types by storing them in a set.
    std::set<std::type_index> types;
    std::transform(tools.begin(), tools.end(),
                   std::inserter(types, types.begin()),
                   [](const auto &tool){
                       return std::type_index(typeid(*tool.get())); });
    EXPECT_EQ(kExpectedToolCount, types.size());
}
