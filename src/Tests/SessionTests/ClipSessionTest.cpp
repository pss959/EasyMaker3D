#include "App/SceneContext.h"
#include "Base/Tuning.h"
#include "Math/Types.h"
#include "Managers/CommandManager.h"
#include "Models/ClippedModel.h"
#include "Models/RootModel.h"
#include "SessionTests/SessionTestBase.h"
#include "Testing.h"
#include "Util/General.h"

// Convenience function
static Rotationf GetXRot_() {
    return Rotationf::FromAxisAndAngle(Vector3f::AxisX(),
                                       Anglef::FromDegrees(90));
}

TEST_F(SessionTestBase, ClipSessionTest1) {
    // This has 1 cylinder (Cylinder_1) that is scaled by 2 in height, rotated
    // by 90 degrees around X, and rtanslated by 5 in X, then converted to a
    // ClippedModel (Clipped_1). Clipped_1 is then clipped by the XY plane in
    // stage coordinates.
    //
    // Clipped_1 should have its rotated top half (front) clipped away.
    LoadSession("Clip1.mvr");

    // Do this twice - the second time after undo/redo.
    for (int i = 0; i < 2; ++i) {
        SCOPED_TRACE(i == 0 ? "First Iteration" : "Second Iteration");

        const auto &rm = *context.scene_context->root_model;
        EXPECT_EQ(1U, rm.GetChildModelCount());
        const auto cm1 = Util::CastToDerived<ClippedModel>(rm.GetChildModel(0));
        ASSERT_TRUE(cm1);
        const auto or1 = cm1->GetOriginalModel();

        const Rotationf xrot = GetXRot_();

        // Original (Cylinder) model transform.
        const float s = TK::kInitialModelScale;
        EXPECT_VECS_CLOSE(Vector3f(s, 2 * s, s), or1->GetScale());
        EXPECT_ROTS_CLOSE(xrot,                  or1->GetRotation());
        EXPECT_VECS_CLOSE(Vector3f(5,  4, 0),    or1->GetTranslation());

        // Clipping plane. The clip plane is the Z=0 plane in stage
        // coordinates, with the normal pointing along +Z. The plane in the
        // ClippedModel is in local coordinates.
        EXPECT_EQ(1U, cm1->GetPlanes().size());
        EXPECT_NEAR(0.f, cm1->GetPlanes()[0].distance, kClose);
        EXPECT_VECS_CLOSE(Vector3f::AxisZ(), cm1->GetPlanes()[0].normal);

        // ClippedModel transform. Translation is adjusted to compensate for
        // the clipped mesh offset.
        EXPECT_VECS_CLOSE(Vector3f(1, 1, 1),     cm1->GetScale());
        EXPECT_ROTS_CLOSE(Rotationf::Identity(), cm1->GetRotation());
        EXPECT_VECS_CLOSE(Vector3f(5,  4, -2),   cm1->GetTranslation());

        // (Object) Bounds.
        const Bounds b1 = cm1->GetBounds();
        EXPECT_PTS_CLOSE(Point3f(0, 0, 0),   b1.GetCenter());
        EXPECT_VECS_CLOSE(Vector3f(4, 4, 4), b1.GetSize());

        // Undo and redo the clip change for second iteration.
        context.command_manager->Undo();
        context.command_manager->Redo();
    }
}

TEST_F(SessionTestBase, ClipSessionTest3) {
    // This has 3 cylinders converted to ClippedModels:

    //  - Cylinder_1, Cylinder_2, and Cylinder_3 are all scaled by 2 in
    //    height. Cylinder_1 is at the origin, Cylinder_2 is translated by 5 in
    //    X and Cylinder_3 is translated by 10 in X.
    //
    //  - Cylinder_2 is rotated by 90 degrees around X.
    //
    //  - All 3 are converted to get Clipped_1, Clipped_2, and Clipped_3.
    //
    //  - Clipped_3 is rotated by 90 degrees around X.
    //
    //  - All 3 ClippedModels are clipped by the XY plane (in stage
    //    coordinates).
    //
    // Therefore:
    //   - Clipped_1 should have its front half clipped away.
    //   - Clipped_2 should have its rotated top half clipped away.
    //   - Clipped_3 should have its unrotated front half clipped away.
    //
    // All resulting ClippedModel remnants should have the same position as
    // before clipping.

    LoadSession("Clip3.mvr");

    // Do this twice - the second time after undo/redo.
    for (int i = 0; i < 2; ++i) {
        SCOPED_TRACE(i == 0 ? "First Iteration" : "Second Iteration");

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
            Rotationf::FromAxisAndAngle(Vector3f::AxisX(),
                                        Anglef::FromDegrees(90));

        // Original (Cylinder) model transforms.
        const float s = TK::kInitialModelScale;
        EXPECT_VECS_CLOSE(Vector3f(s, 2 * s, s), or1->GetScale());
        EXPECT_VECS_CLOSE(Vector3f(s, 2 * s, s), or2->GetScale());
        EXPECT_VECS_CLOSE(Vector3f(s, 2 * s, s), or3->GetScale());
        EXPECT_ROTS_CLOSE(Rotationf::Identity(), or1->GetRotation());
        EXPECT_ROTS_CLOSE(xrot,                  or2->GetRotation());
        EXPECT_ROTS_CLOSE(Rotationf::Identity(), or3->GetRotation());
        EXPECT_VECS_CLOSE(Vector3f(0,  4, 0),    or1->GetTranslation());
        EXPECT_VECS_CLOSE(Vector3f(5,  4, 0),    or2->GetTranslation());
        EXPECT_VECS_CLOSE(Vector3f(10, 4, 0),    or3->GetTranslation());

        // Clipping planes. The clip plane is the Z=0 plane in stage
        // coordinates, with the normal pointing along +Z. The planes in the
        // ClippedModels are in local coordinates, so they should all be the
        // same.
        EXPECT_EQ(1U, cm1->GetPlanes().size());
        EXPECT_EQ(1U, cm2->GetPlanes().size());
        EXPECT_EQ(1U, cm3->GetPlanes().size());
        EXPECT_NEAR(0.f, cm1->GetPlanes()[0].distance, kClose);
        EXPECT_NEAR(0.f, cm2->GetPlanes()[0].distance, kClose);
        EXPECT_NEAR(0.f, cm3->GetPlanes()[0].distance, kClose);
        EXPECT_VECS_CLOSE(Vector3f::AxisZ(), cm1->GetPlanes()[0].normal);
        EXPECT_VECS_CLOSE(Vector3f::AxisZ(), cm2->GetPlanes()[0].normal);
        EXPECT_VECS_CLOSE(Vector3f::AxisZ(), cm3->GetPlanes()[0].normal);

        // Clipped models transforms. Translations are adjusted to compensate
        // for the clipped mesh offsets.
        EXPECT_VECS_CLOSE(Vector3f(1, 1, 1),     cm1->GetScale());
        EXPECT_VECS_CLOSE(Vector3f(1, 1, 1),     cm2->GetScale());
        EXPECT_VECS_CLOSE(Vector3f(1, 1, 1),     cm3->GetScale());
        EXPECT_ROTS_CLOSE(Rotationf::Identity(), cm1->GetRotation());
        EXPECT_ROTS_CLOSE(Rotationf::Identity(), cm2->GetRotation());
        EXPECT_ROTS_CLOSE(xrot,                  cm3->GetRotation());
        EXPECT_VECS_CLOSE(Vector3f(0,  4, -1),   cm1->GetTranslation());
        EXPECT_VECS_CLOSE(Vector3f(5,  4, -2),   cm2->GetTranslation());
        EXPECT_VECS_CLOSE(Vector3f(10, 5,  0),   cm3->GetTranslation());

        // (Object) Bounds.
        const Bounds b1 = cm1->GetBounds();
        const Bounds b2 = cm2->GetBounds();
        const Bounds b3 = cm3->GetBounds();
        EXPECT_PTS_CLOSE(Point3f(0, 0, 0),   b1.GetCenter());
        EXPECT_PTS_CLOSE(Point3f(0, 0, 0),   b2.GetCenter());
        EXPECT_PTS_CLOSE(Point3f(0, 0, 0),   b3.GetCenter());
        EXPECT_VECS_CLOSE(Vector3f(4, 8, 2), b1.GetSize());
        EXPECT_VECS_CLOSE(Vector3f(4, 4, 4), b2.GetSize());
        EXPECT_VECS_CLOSE(Vector3f(4, 8, 2), b3.GetSize());

        // Undo and redo the clip change for second iteration.
        context.command_manager->Undo();
        context.command_manager->Redo();
    }
}
