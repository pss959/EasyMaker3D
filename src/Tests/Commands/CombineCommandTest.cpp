//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/CombineCSGModelCommand.h"
#include "Commands/CombineHullModelCommand.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// This tests the base CombineCommand and all classes derived from it. The
/// tests are almost identical, so a function in the test class does most of
/// the work.

/// \ingroup Tests
class CombineCommandTest : public CommandTestBase {
  protected:
    /// Tests the named command.
    void TestCommand(const Str &type_name, const Str &action);
};

TEST_F(CombineCommandTest, ResultName) {
    // Need to use a derived concrete class here.
    auto chmc = Command::CreateCommand<CombineHullModelCommand>();
    EXPECT_TRUE(chmc->GetResultName().empty());
    chmc->SetResultName("Fred");
    EXPECT_EQ("Fred", chmc->GetResultName());
}

TEST_F(CombineCommandTest, CombineCSGModelCommand) {
    auto ccmc = Command::CreateCommand<CombineCSGModelCommand>();
    EXPECT_EQ(CSGOperation::kUnion, ccmc->GetOperation());
    ccmc->SetOperation(CSGOperation::kIntersection);
    EXPECT_EQ(CSGOperation::kIntersection, ccmc->GetOperation());

    SetParseTypeName("CombineCSGModelCommand");
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [" BadName"])", "Invalid model name");
    TestInvalid(R"(model_names: ["A", "B"])", "Invalid result model name");
    TestInvalid(R"(model_names: ["A", "B"], result_name: " X")",
                "Invalid result model name");
    TestInvalid(R"(model_names: ["A"], result_name: "B")",
                "Need at least two model names");
    TestValid(R"(model_names: ["A", "B"], result_name: "C")");
    TestDesc(R"(model_names: ["A", "B"], result_name: "C")",
             R"(Created CSG model "C" from 2 Models)");
    TestDesc(R"(model_names: ["A", "B", "C"], result_name: "D")",
             R"(Created CSG model "D" from 3 Models)");
}

TEST_F(CombineCommandTest, CombineHullModelCommand) {
    SetParseTypeName("CombineHullModelCommand");
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [" BadName"])", "Invalid model name");
    TestInvalid(R"(model_names: ["A", "B"])", "Invalid result model name");
    TestInvalid(R"(model_names: ["A", "B"], result_name: " X")",
                "Invalid result model name");
    TestValid(R"(model_names: ["A"], result_name: "B")");
    TestValid(R"(model_names: ["A", "B"], result_name: "C")");
    TestDesc(R"(model_names: ["A"], result_name: "B")",
             R"(Created convex hull model "B" from Model "A")");
    TestDesc(R"(model_names: ["A", "B"], result_name: "C")",
             R"(Created convex hull model "C" from 2 Models)");
    TestDesc(R"(model_names: ["A", "B", "C"], result_name: "D")",
             R"(Created convex hull model "D" from 3 Models)");
}
