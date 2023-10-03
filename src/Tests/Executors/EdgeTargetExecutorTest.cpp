#include "Commands/ChangeEdgeTargetCommand.h"
#include "Executors/EdgeTargetExecutor.h"
#include "Managers/TargetManager.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class EdgeTargetExecutorTest : public ExecutorTestBase {};

TEST_F(EdgeTargetExecutorTest, TypeName) {
    EdgeTargetExecutor exec;
    EXPECT_EQ("ChangeEdgeTargetCommand", exec.GetCommandTypeName());
}

TEST_F(EdgeTargetExecutorTest, ChangeCylinder) {
    auto cmd = ParseCommand<ChangeEdgeTargetCommand>(
        "ChangeEdgeTargetCommand",
        R"(old_target: EdgeTarget {},
           new_target: EdgeTarget { position0: 6 5 2, position1: 1 5 2 })");

    EdgeTargetExecutor exec;
    auto context = InitContext(exec, true);  // Need targets.

    // Default target
    EXPECT_FALSE(context.target_manager->IsEdgeTargetVisible());
    const auto &et = context.target_manager->GetEdgeTarget();
    EXPECT_EQ(Point3f::Zero(),   et.GetPosition0());
    EXPECT_EQ(Point3f(0, 4, 0),  et.GetPosition1());
    EXPECT_EQ(Vector3f::AxisY(), et.GetDirection());
    EXPECT_EQ(4,                 et.GetLength());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_TRUE(context.target_manager->IsEdgeTargetVisible());
    EXPECT_EQ(Point3f(6, 5, 2),   et.GetPosition0());
    EXPECT_EQ(Point3f(1, 5, 2),   et.GetPosition1());
    EXPECT_EQ(-Vector3f::AxisX(), et.GetDirection());
    EXPECT_EQ(5,                  et.GetLength());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_TRUE(context.target_manager->IsEdgeTargetVisible());
    EXPECT_EQ(Point3f::Zero(),   et.GetPosition0());
    EXPECT_EQ(Point3f(0, 4, 0),  et.GetPosition1());
    EXPECT_EQ(Vector3f::AxisY(), et.GetDirection());
    EXPECT_EQ(4,                 et.GetLength());
}
