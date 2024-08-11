//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangePointTargetCommand.h"
#include "Executors/PointTargetExecutor.h"
#include "Managers/TargetManager.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class PointTargetExecutorTest : public ExecutorTestBase {};

TEST_F(PointTargetExecutorTest, TypeName) {
    PointTargetExecutor exec;
    EXPECT_EQ("ChangePointTargetCommand", exec.GetCommandTypeName());
}

TEST_F(PointTargetExecutorTest, ChangeCylinder) {
    auto cmd = ParseCommand<ChangePointTargetCommand>(
        "ChangePointTargetCommand",
        R"(old_target: PointTarget {},
           new_target: PointTarget { position: 1 2 3, direction: 0 0 -1,
                                     radius: 3.5, arc: 45 -270 })");

    PointTargetExecutor exec;
    auto context = InitContext(exec, true);  // Need targets.

    // Default target
    EXPECT_FALSE(context.target_manager->IsPointTargetVisible());
    const auto &pt = context.target_manager->GetPointTarget();
    EXPECT_EQ(Point3f::Zero(),    pt.GetPosition());
    EXPECT_EQ(Vector3f::AxisY(),  pt.GetDirection());
    EXPECT_EQ(1,                  pt.GetRadius());
    EXPECT_EQ(CircleArc(),        pt.GetArc());

    CircleArc arc(Anglef::FromDegrees(45), Anglef::FromDegrees(-270));

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_TRUE(context.target_manager->IsPointTargetVisible());
    EXPECT_EQ(Point3f(1, 2, 3),   pt.GetPosition());
    EXPECT_EQ(-Vector3f::AxisZ(), pt.GetDirection());
    EXPECT_EQ(3.5f,               pt.GetRadius());
    EXPECT_EQ(arc,                pt.GetArc());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_TRUE(context.target_manager->IsPointTargetVisible());
    EXPECT_EQ(Point3f::Zero(),    pt.GetPosition());
    EXPECT_EQ(Vector3f::AxisY(),  pt.GetDirection());
    EXPECT_EQ(1,                  pt.GetRadius());
    EXPECT_EQ(CircleArc(),        pt.GetArc());
}
