//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ConvertTaperCommand.h"
#include "Executors/ConvertTaperExecutor.h"
#include "Tests/Executors/ConvertExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ConvertTaperExecutorTest : public ConvertExecutorTestBase {};

TEST_F(ConvertTaperExecutorTest, TypeName) {
    ConvertTaperExecutor exec;
    EXPECT_EQ("ConvertTaperCommand", exec.GetCommandTypeName());
}

TEST_F(ConvertTaperExecutorTest, ConvertTaperWithoutName) {
    // Set up a ConvertTaperCommand without result names. This cannot be created
    // by parsing because having no result names is a parse error.
    auto cmd = Command::CreateCommand<ConvertTaperCommand>();
    ConvertTaperExecutor exec;
    ConvertWithoutNames(*cmd, exec, "TaperedModel", "Tapered");
}

TEST_F(ConvertTaperExecutorTest, ConvertTaperWithName) {
    auto cmd = ParseCommand<ConvertTaperCommand>(
        "ConvertTaperCommand",
        R"(model_names: ["Box_0", "Box_1"],
           result_names: ["Tapered_0", "Tapered_1"])");
    ConvertTaperExecutor exec;
    ConvertWithNames(*cmd, exec, "TaperedModel", "Tapered");
}
