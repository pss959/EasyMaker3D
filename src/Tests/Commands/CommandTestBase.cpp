#include "Tests/Commands/CommandTestBase.h"

#include "Commands/Command.h"

void CommandTestBase::TestDesc(const std::string &str,
                               const std::string &desc) {
    auto cmd = ParseTypedObject<Command>(str);
    EXPECT_EQ(desc, cmd->GetDescription());
}
