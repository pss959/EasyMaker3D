#include "Commands/CommandList.h"
#include "Managers/CommandManager.h"
#include "Math/Types.h"
#include "Models/RootModel.h"
#include "SceneContext.h"
#include "SessionTests/SessionTestBase.h"
#include "Testing.h"

TEST_F(SessionTestBase, TransformSessionTest) {
    LoadSession("Transforms.mvr");

    const auto &rm = *context.scene_context->root_model;
    EXPECT_EQ(3U, rm.GetChildModelCount());

    // Box_1 is translated by 10 in x and scaled to 20 units in Y.
    const auto &box1 = *rm.GetChildModel(0);
    EXPECT_EQ("Box_1", box1.GetName());
    EXPECT_EQ(Vector3f(4, 10, 4),  box1.GetScale());
    EXPECT_EQ(Vector3f(10, 10, 0), box1.GetTranslation());
    EXPECT_TRUE(box1.GetRotation().IsIdentity());

    // Box_2 is translated by -10 in x.
    const auto &box2 = *rm.GetChildModel(1);
    EXPECT_EQ("Box_2", box2.GetName());
    EXPECT_EQ(Vector3f(4, 4, 4),   box2.GetScale());
    EXPECT_EQ(Vector3f(-10, 4, 0), box2.GetTranslation());
    EXPECT_TRUE(box2.GetRotation().IsIdentity());

    // Box_3 is rotated around x by 90 degrees and translated by -10 in Z.
    const auto &box3 = *rm.GetChildModel(2);
    EXPECT_EQ("Box_3", box3.GetName());
    EXPECT_EQ(Vector3f(4, 4, 4),  box3.GetScale());
    EXPECT_VECS_CLOSE(Vector3f(0, 4, -10), box3.GetTranslation());
    EXPECT_ROTS_CLOSE(Rotationf::FromAxisAndAngle(Vector3f::AxisX(),
                                                  Anglef::FromDegrees(90)),
                      box3.GetRotation());

    // XXXX Check transforms
}
