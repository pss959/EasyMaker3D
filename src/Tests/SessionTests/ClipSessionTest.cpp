#include "App/SceneContext.h"
#include "Base/Tuning.h"
#include "Math/Types.h"
#include "Models/ClippedModel.h"
#include "Models/RootModel.h"
#include "SessionTests/SessionTestBase.h"
#include "Testing.h"
#include "Util/General.h"

TEST_F(SessionTestBase, ClipSessionTest) {
    // This has 3 cylinders converted to ClippedModels:
    //  - Cylinder_1 / Clipped_1 is untransformed. It should have the front
    //      half clipped away.
    //  - Cylinder_2 is rotated 90 around X and translated by 5 in X
    //      BEFORE converting to Clipped_2. It should have the (rotated) front
    //      half clipped away.
    //  - Cylinder_3 is translated by 10 in X BEFORE converting to Clipped_3
    //      and rotated 90 around X AFTER converting. It should have the
    //      (unrotated) front half clipped away, so it should be a half
    //      cylinder floating above the stage with the flat part down.
    LoadSession("Clip.mvr");

    const auto &rm = *context.scene_context->root_model;
    EXPECT_EQ(3U, rm.GetChildModelCount());
    const auto cm1 = Util::CastToDerived<ClippedModel>(rm.GetChildModel(0));
    const auto cm2 = Util::CastToDerived<ClippedModel>(rm.GetChildModel(1));
    const auto cm3 = Util::CastToDerived<ClippedModel>(rm.GetChildModel(2));
    ASSERT_TRUE(cm1);
    ASSERT_TRUE(cm2);
    ASSERT_TRUE(cm3);
    const auto or1 = cm1->GetOriginalModel();
    const auto or2 = cm2->GetOriginalModel();
    const auto or3 = cm3->GetOriginalModel();

    const Rotationf xrot =
        Rotationf::FromAxisAndAngle(Vector3f::AxisX(), Anglef::FromDegrees(90));

    // Original (Cylinder) model transforms.
    const float s = TK::kInitialModelScale;
    EXPECT_VECS_CLOSE(Vector3f(s, s, s),     or1->GetScale());
    EXPECT_VECS_CLOSE(Vector3f(s, s, s),     or2->GetScale());
    EXPECT_VECS_CLOSE(Vector3f(s, s, s),     or3->GetScale());
    EXPECT_ROTS_CLOSE(Rotationf::Identity(), or1->GetRotation());
    EXPECT_ROTS_CLOSE(xrot,                  or2->GetRotation());
    EXPECT_ROTS_CLOSE(Rotationf::Identity(), or3->GetRotation());
    EXPECT_VECS_CLOSE(Vector3f(0,  2, 0),    or1->GetTranslation());
    EXPECT_VECS_CLOSE(Vector3f(5,  2, 0),    or2->GetTranslation());
    EXPECT_VECS_CLOSE(Vector3f(10, 2, 0),    or3->GetTranslation());

    // Clipped models transforms.
    EXPECT_VECS_CLOSE(Vector3f(1, 1, 1),     cm1->GetScale());
    EXPECT_VECS_CLOSE(Vector3f(1, 1, 1),     cm2->GetScale());
    EXPECT_VECS_CLOSE(Vector3f(1, 1, 1),     cm3->GetScale());
    EXPECT_ROTS_CLOSE(Rotationf::Identity(), cm1->GetRotation());
    EXPECT_ROTS_CLOSE(Rotationf::Identity(), cm2->GetRotation());
    EXPECT_ROTS_CLOSE(xrot,                  cm3->GetRotation());
    EXPECT_VECS_CLOSE(Vector3f(0,  2, -1),   cm1->GetTranslation());
    EXPECT_VECS_CLOSE(Vector3f(5,  2, -1),   cm2->GetTranslation());
    EXPECT_VECS_CLOSE(Vector3f(10, 2, -1),   cm3->GetTranslation());

    // Clipping planes. The clip plane is the Z=0 plane in stage coordinates,
    // with the normal pointing along +Z. The planes in the ClippedModels are
    // in local coordinates, so they should all be the same.
    EXPECT_EQ(1U, cm1->GetPlanes().size());
    EXPECT_EQ(1U, cm2->GetPlanes().size());
    EXPECT_EQ(1U, cm3->GetPlanes().size());
    EXPECT_NEAR(0.f, cm1->GetPlanes()[0].distance, kClose);
    EXPECT_NEAR(0.f, cm2->GetPlanes()[0].distance, kClose);
    EXPECT_NEAR(0.f, cm3->GetPlanes()[0].distance, kClose);
    EXPECT_VECS_CLOSE(Vector3f::AxisZ(), cm1->GetPlanes()[0].normal);
    EXPECT_VECS_CLOSE(Vector3f::AxisZ(), cm2->GetPlanes()[0].normal);
    EXPECT_VECS_CLOSE(Vector3f::AxisZ(), cm3->GetPlanes()[0].normal);

    // (Object) Bounds should all be the same.
    const Bounds b1 = cm1->GetBounds();
    const Bounds b2 = cm2->GetBounds();
    const Bounds b3 = cm3->GetBounds();
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),  b1.GetCenter());
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),  b2.GetCenter());
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),  b3.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(4, 4, 2), b1.GetSize());
    EXPECT_VECS_CLOSE(Vector3f(4, 4, 2), b2.GetSize());
    EXPECT_VECS_CLOSE(Vector3f(4, 4, 2), b3.GetSize());
}
