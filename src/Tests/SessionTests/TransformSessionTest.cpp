#include "App/SceneContext.h"
#include "Commands/CommandList.h"
#include "Managers/CommandManager.h"
#include "Math/Types.h"
#include "Models/RootModel.h"
#include "SessionTests/SessionTestBase.h"
#include "Testing.h"

TEST_F(SessionTestBase, TransformSessionTest) {
    LoadSession("Transforms.mvr");

    const auto &rm = *context.scene_context->root_model;
    EXPECT_EQ(5U, rm.GetChildModelCount());
    const auto &box1 = *rm.GetChildModel(0);
    const auto &box2 = *rm.GetChildModel(1);
    const auto &box3 = *rm.GetChildModel(2);
    const auto &box4 = *rm.GetChildModel(3);
    const auto &box5 = *rm.GetChildModel(4);

    // Box_1 is translated by 10 in X and scaled asymmetrically to 20 units in
    // Y.
    EXPECT_EQ("Box_1", box1.GetName());
    EXPECT_EQ(Vector3f(4, 10, 4),  box1.GetScale());
    EXPECT_EQ(Vector3f(10, 10, 0), box1.GetTranslation());
    EXPECT_TRUE(box1.GetRotation().IsIdentity());

    // Box_2 is translated by -10 in X.
    EXPECT_EQ("Box_2", box2.GetName());
    EXPECT_EQ(Vector3f(4, 4, 4),   box2.GetScale());
    EXPECT_EQ(Vector3f(-10, 4, 0), box2.GetTranslation());
    EXPECT_TRUE(box2.GetRotation().IsIdentity());

    // Box_3 is rotated around X by 90 degrees and translated by -10 in Z.
    EXPECT_EQ("Box_3", box3.GetName());
    EXPECT_EQ(Vector3f(4, 4, 4),  box3.GetScale());
    EXPECT_VECS_CLOSE(Vector3f(0, 4, -10), box3.GetTranslation());
    EXPECT_ROTS_CLOSE(Rotationf::FromAxisAndAngle(Vector3f::AxisX(),
                                                  Anglef::FromDegrees(90)),
                      box3.GetRotation());

    // Box_4 is rotated around Z by 90 degrees, scaled asymmetrically to 20
    // units in X (looks like Y), and translated by 10 in Z.
    EXPECT_EQ("Box_4", box4.GetName());
    EXPECT_EQ(Vector3f(10, 4, 4),  box4.GetScale());
    EXPECT_VECS_CLOSE(Vector3f(0, 10, 10), box4.GetTranslation());
    EXPECT_ROTS_CLOSE(Rotationf::FromAxisAndAngle(Vector3f::AxisZ(),
                                                  Anglef::FromDegrees(90)),
                      box4.GetRotation());

    // Box_5 is rotated around Y by 90 degrees, then rotated 90 degrees around
    // X (looks like Z). The +X axis should now be pointing up.
    EXPECT_EQ("Box_5", box5.GetName());
    EXPECT_EQ(Vector3f(4, 4, 4), box5.GetScale());
    EXPECT_VECS_CLOSE(Vector3f(0, 4, 0), box5.GetTranslation());
    EXPECT_ROTS_CLOSE(Rotationf::RotateInto(Vector3f::AxisX(),
                                            Vector3f::AxisY()),
                      box4.GetRotation());
}
