#include <algorithm>
#include <set>
#include <typeinfo>

#include "Executors/InitExecutors.h"
#include "Tests/Testing.h"

TEST(InitExecutorsTest, InitExecutors) {
    const size_t kExpectedExecutorCount = 40;

    // Test that the number of Executor types is the expected amount.
    const auto execs = InitExecutors();
    EXPECT_EQ(kExpectedExecutorCount, execs.size());

    // Test that there are no duplicate types by storing them in a set.
    std::set<std::type_index> types;
    std::transform(execs.begin(), execs.end(),
                   std::inserter(types, types.begin()),
                   [](const auto &exec){
                       return std::type_index(typeid(*exec.get())); });
    EXPECT_EQ(kExpectedExecutorCount, types.size());
}
