#include "Tests/Commands/CommandTestBase.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "Commands/Command.h"

void CommandTestBase::TestDesc(const std::string &str,
                               const std::string &desc_start) {
    auto cmd = ParseTypedObject<Command>(str);
    EXPECT_THAT(cmd->GetDescription(), ::testing::StartsWith(desc_start));
}
