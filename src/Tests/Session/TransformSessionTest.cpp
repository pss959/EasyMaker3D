//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/CommandList.h"
#include "Managers/CommandManager.h"
#include "Managers/SceneContext.h"
#include "Math/Types.h"
#include "Models/RootModel.h"
#include "Tests/Session/SessionTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class TransformSessionTest : public SessionTestBase {};

TEST_F(TransformSessionTest, FiveBoxes) {
    LoadSession("Transforms");

    const auto &rm = *context.scene_context->root_model;
    EXPECT_EQ(5U, rm.GetChildModelCount());
    const auto &box1 = *rm.GetChildModel(0);
    const auto &box2 = *rm.GetChildModel(1);
    const auto &box3 = *rm.GetChildModel(2);
    const auto &box4 = *rm.GetChildModel(3);
    const auto &box5 = *rm.GetChildModel(4);

    // Box_1 is translated by 10 in X and scaled asymmetrically by 2.5x in Y.
    EXPECT_EQ("Box_1", box1.GetName());
    EXPECT_EQ(Vector3f(MS, 2.5f * MS, MS), box1.GetScale());
    EXPECT_EQ(Vector3f(10, 2.5f * MS, 0),  box1.GetTranslation());
    EXPECT_TRUE(box1.GetRotation().IsIdentity());

    // Box_2 is translated by -10 in X.
    EXPECT_EQ("Box_2", box2.GetName());
    EXPECT_EQ(Vector3f(MS, MS, MS), box2.GetScale());
    EXPECT_EQ(Vector3f(-10, MS, 0), box2.GetTranslation());
    EXPECT_TRUE(box2.GetRotation().IsIdentity());

    // Box_3 is rotated around X by 90 degrees and translated by -10 in Z.
    EXPECT_EQ("Box_3", box3.GetName());
    EXPECT_EQ(Vector3f(MS, MS, MS),  box3.GetScale());
    EXPECT_VECS_CLOSE(Vector3f(0, MS, -10), box3.GetTranslation());
    EXPECT_ROTS_CLOSE(BuildRotation(Vector3f::AxisX(), 90), box3.GetRotation());

    // Box_4 is rotated around Z by 90 degrees, scaled asymmetrically by 2.5x
    // in X (looks like Y), and translated by 10 in Z.
    EXPECT_EQ("Box_4", box4.GetName());
    EXPECT_EQ(Vector3f(2.5f * MS, MS, MS),  box4.GetScale());
    EXPECT_VECS_CLOSE(Vector3f(0, 2.5f * MS, 10), box4.GetTranslation());
    EXPECT_ROTS_CLOSE(BuildRotation(Vector3f::AxisZ(), 90), box4.GetRotation());

    // Box_5 is rotated around Y by 90 degrees, then rotated 90 degrees around
    // X (looks like Z). The +X axis should now be pointing up.
    EXPECT_EQ("Box_5", box5.GetName());
    EXPECT_EQ(Vector3f(MS, MS, MS), box5.GetScale());
    EXPECT_VECS_CLOSE(Vector3f(0, MS, 0), box5.GetTranslation());
    EXPECT_ROTS_CLOSE(Rotationf::RotateInto(Vector3f::AxisX(),
                                            Vector3f::AxisY()),
                      box4.GetRotation());
}
