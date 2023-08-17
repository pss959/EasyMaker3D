#include "Tests/Commands/CommandTestBase.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "Commands/Command.h"

void CommandTestBase::TestDesc(const Str &contents, const Str &desc_start) {
    ASSERT_FALSE(GetParseTypeName().empty());
    auto cmd = ParseTypedObject<Command>(BuildParseString(contents));
    EXPECT_THAT(cmd->GetDescription(), ::testing::StartsWith(desc_start));
}
