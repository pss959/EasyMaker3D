#include "Managers/SceneContext.h"
#include "Math/Types.h"
#include "Models/MirroredModel.h"
#include "Models/RootModel.h"
#include "Tests/Session/SessionTestBase.h"
#include "Tests/Testing.h"
#include "Util/Tuning.h"

class MirrorSessionTest : public SessionTestBase {};

TEST_F(MirrorSessionTest, ThreeCylinders) {
    // This has 3 cylinders converted to MirroredModels:
    //  - Cylinder_1 / Mirrored_1 is untransformed.
    //  - Cylinder_2 is rotated 90 around X and translated by 5 in X
    //      BEFORE converting to Mirrored_2.
    //  - Cylinder_3 is translated by 10 in X BEFORE converting to Mirrored_3
    //      and rotated 90 around X AFTER converting.
    LoadSession("Mirror");

    const auto &rm = *context.scene_context->root_model;
    EXPECT_EQ(3U, rm.GetChildModelCount());
    const auto mm1 =
        std::dynamic_pointer_cast<MirroredModel>(rm.GetChildModel(0));
    const auto mm2 =
        std::dynamic_pointer_cast<MirroredModel>(rm.GetChildModel(1));
    const auto mm3 =
        std::dynamic_pointer_cast<MirroredModel>(rm.GetChildModel(2));
    ASSERT_TRUE(mm1);
    ASSERT_TRUE(mm2);
    ASSERT_TRUE(mm3);
    const auto op1 = mm1->GetOperandModel();
    const auto op2 = mm2->GetOperandModel();
    const auto op3 = mm3->GetOperandModel();

    const Rotationf xrot = BuildRotation(Vector3f::AxisX(), 90);

    // Operand (Cylinder) model transforms.
    const float s = TK::kInitialModelScale;
    EXPECT_VECS_CLOSE(Vector3f(s, s, s),     op1->GetScale());
    EXPECT_VECS_CLOSE(Vector3f(s, s, s),     op2->GetScale());
    EXPECT_VECS_CLOSE(Vector3f(s, s, s),     op3->GetScale());
    EXPECT_ROTS_CLOSE(Rotationf::Identity(), op1->GetRotation());
    EXPECT_ROTS_CLOSE(xrot,                  op2->GetRotation());
    EXPECT_ROTS_CLOSE(Rotationf::Identity(), op3->GetRotation());
    EXPECT_VECS_CLOSE(Vector3f(0,  2, 0),    op1->GetTranslation());
    EXPECT_VECS_CLOSE(Vector3f(5,  2, 0),    op2->GetTranslation());
    EXPECT_VECS_CLOSE(Vector3f(10, 2, 0),    op3->GetTranslation());

    // Mirrored models transforms. Translations should be mirrored across the
    // stage plane.
    EXPECT_VECS_CLOSE(Vector3f(s, s, s),     mm1->GetScale());
    EXPECT_VECS_CLOSE(Vector3f(s, s, s),     mm2->GetScale());
    EXPECT_VECS_CLOSE(Vector3f(s, s, s),     mm3->GetScale());
    EXPECT_ROTS_CLOSE(Rotationf::Identity(), mm1->GetRotation());
    EXPECT_ROTS_CLOSE(xrot,                  mm2->GetRotation());
    EXPECT_ROTS_CLOSE(xrot,                  mm3->GetRotation());
    EXPECT_VECS_CLOSE(Vector3f(0,   2, 0),   mm1->GetTranslation());
    EXPECT_VECS_CLOSE(Vector3f(-5,  2, 0),   mm2->GetTranslation());
    EXPECT_VECS_CLOSE(Vector3f(-10, 2, 0),   mm3->GetTranslation());

    // Mirror planes. The mirror plane is the Z=0 plane in stage coordinates,
    // with the normal pointing along +X. The planes in the MirroredModels are
    // in object coordinates.
    EXPECT_VECS_CLOSE(Vector3f::AxisX(), mm1->GetPlaneNormal());
    EXPECT_VECS_CLOSE(Vector3f::AxisX(), mm2->GetPlaneNormal());
    EXPECT_VECS_CLOSE(Vector3f::AxisX(), mm3->GetPlaneNormal());

    // (Object) Bounds should all be the same.
    const Bounds b1 = mm1->GetBounds();
    const Bounds b2 = mm2->GetBounds();
    const Bounds b3 = mm3->GetBounds();
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),  b1.GetCenter());
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),  b2.GetCenter());
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),  b3.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(2, 2, 2), b1.GetSize());
    EXPECT_VECS_CLOSE(Vector3f(2, 2, 2), b2.GetSize());
    EXPECT_VECS_CLOSE(Vector3f(2, 2, 2), b3.GetSize());
}
