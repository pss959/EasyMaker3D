//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ConvertClipCommand.h"
#include "Executors/ConvertClipExecutor.h"
#include "Tests/Executors/ConvertExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ConvertClipExecutorTest : public ConvertExecutorTestBase {};

TEST_F(ConvertClipExecutorTest, TypeName) {
    ConvertClipExecutor exec;
    EXPECT_EQ("ConvertClipCommand", exec.GetCommandTypeName());
}

TEST_F(ConvertClipExecutorTest, ConvertClipWithoutName) {
    // Set up a ConvertClipCommand without result names. This cannot be created
    // by parsing because having no result names is a parse error.
    auto cmd = Command::CreateCommand<ConvertClipCommand>();
    ConvertClipExecutor exec;
    ConvertWithoutNames(*cmd, exec, "ClippedModel", "Clipped");
}

TEST_F(ConvertClipExecutorTest, ConvertClipWithName) {
    auto cmd = ParseCommand<ConvertClipCommand>(
        "ConvertClipCommand",
        R"(model_names: ["Box_0", "Box_1"],
           result_names: ["Clipped_0", "Clipped_1"])");
    ConvertClipExecutor exec;
    ConvertWithNames(*cmd, exec, "ClippedModel", "Clipped");
}
