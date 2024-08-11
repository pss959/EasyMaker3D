//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ConvertBevelCommand.h"
#include "Executors/ConvertBevelExecutor.h"
#include "Tests/Executors/ConvertExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ConvertBevelExecutorTest : public ConvertExecutorTestBase {};

TEST_F(ConvertBevelExecutorTest, TypeName) {
    ConvertBevelExecutor exec;
    EXPECT_EQ("ConvertBevelCommand", exec.GetCommandTypeName());
}

TEST_F(ConvertBevelExecutorTest, ConvertBevelWithoutName) {
    // Set up a ConvertBevelCommand without result names. This cannot be created
    // by parsing because having no result names is a parse error.
    auto cmd = Command::CreateCommand<ConvertBevelCommand>();
    ConvertBevelExecutor exec;
    ConvertWithoutNames(*cmd, exec, "BeveledModel", "Beveled");
}

TEST_F(ConvertBevelExecutorTest, ConvertBevelWithName) {
    auto cmd = ParseCommand<ConvertBevelCommand>(
        "ConvertBevelCommand",
        R"(model_names: ["Box_0", "Box_1"],
           result_names: ["Beveled_0", "Beveled_1"])");
    ConvertBevelExecutor exec;
    ConvertWithNames(*cmd, exec, "BeveledModel", "Beveled");
}
