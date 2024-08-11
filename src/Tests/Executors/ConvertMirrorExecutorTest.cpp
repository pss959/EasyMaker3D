//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ConvertMirrorCommand.h"
#include "Executors/ConvertMirrorExecutor.h"
#include "Tests/Executors/ConvertExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ConvertMirrorExecutorTest : public ConvertExecutorTestBase {};

TEST_F(ConvertMirrorExecutorTest, TypeName) {
    ConvertMirrorExecutor exec;
    EXPECT_EQ("ConvertMirrorCommand", exec.GetCommandTypeName());
}

TEST_F(ConvertMirrorExecutorTest, ConvertMirrorWithoutName) {
    // Set up a ConvertMirrorCommand without result names. This cannot be created
    // by parsing because having no result names is a parse error.
    auto cmd = Command::CreateCommand<ConvertMirrorCommand>();
    ConvertMirrorExecutor exec;
    ConvertWithoutNames(*cmd, exec, "MirroredModel", "Mirrored");
}

TEST_F(ConvertMirrorExecutorTest, ConvertMirrorWithName) {
    auto cmd = ParseCommand<ConvertMirrorCommand>(
        "ConvertMirrorCommand",
        R"(model_names: ["Box_0", "Box_1"],
           result_names: ["Mirrored_0", "Mirrored_1"])");
    ConvertMirrorExecutor exec;
    ConvertWithNames(*cmd, exec, "MirroredModel", "Mirrored");
}
