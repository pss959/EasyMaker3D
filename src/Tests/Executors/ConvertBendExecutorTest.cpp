//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ConvertBendCommand.h"
#include "Executors/ConvertBendExecutor.h"
#include "Tests/Executors/ConvertExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ConvertBendExecutorTest : public ConvertExecutorTestBase {};

TEST_F(ConvertBendExecutorTest, TypeName) {
    ConvertBendExecutor exec;
    EXPECT_EQ("ConvertBendCommand", exec.GetCommandTypeName());
}

TEST_F(ConvertBendExecutorTest, ConvertBendWithoutName) {
    // Set up a ConvertBendCommand without result names. This cannot be created
    // by parsing because having no result names is a parse error.
    auto cmd = Command::CreateCommand<ConvertBendCommand>();
    ConvertBendExecutor exec;
    ConvertWithoutNames(*cmd, exec, "BentModel", "Bent");
}

TEST_F(ConvertBendExecutorTest, ConvertBendWithName) {
    auto cmd = ParseCommand<ConvertBendCommand>(
        "ConvertBendCommand",
        R"(model_names: ["Box_0", "Box_1"],
           result_names: ["Bent_0", "Bent_1"])");
    ConvertBendExecutor exec;
    ConvertWithNames(*cmd, exec, "BentModel", "Bent");
}
