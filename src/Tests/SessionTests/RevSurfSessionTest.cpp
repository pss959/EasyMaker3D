#include "App/SceneContext.h"
#include "Base/Tuning.h"
#include "Math/Types.h"
#include "Managers/CommandManager.h"
#include "Models/RevSurfModel.h"
#include "Models/RootModel.h"
#include "SessionTests/SessionTestBase.h"
#include "Testing.h"
#include "Util/General.h"

// Convenience function
static Rotationf GetXRot_() {
    return Rotationf::FromAxisAndAngle(Vector3f::AxisX(),
                                       Anglef::FromDegrees(90));
}

TEST_F(SessionTestBase, RevSurfSessionTest1) {
    // This has 1 RevSurfModel (RevSurf_1) that is scaled by 2 in height,
    // rotated by 90 degrees around X, and translated by 5 in X.
    LoadSession("RevSurf1.mvr");

    // Do this twice - the second time after undo/redo.
    for (int i = 0; i < 2; ++i) {
        SCOPED_TRACE(i == 0 ? "First Iteration" : "Second Iteration");

        const auto &rm = *context.scene_context->root_model;
        EXPECT_EQ(1U, rm.GetChildModelCount());
        const auto rsm = Util::CastToDerived<RevSurfModel>(rm.GetChildModel(0));
        ASSERT_TRUE(rsm);

        const Rotationf xrot = GetXRot_();

        const float s = TK::kRevSurfHeight;
        EXPECT_VECS_CLOSE(Vector3f(s, 2 * s, s), rsm->GetScale());
        EXPECT_ROTS_CLOSE(xrot,                  rsm->GetRotation());
        EXPECT_VECS_CLOSE(Vector3f(5,  4, 0),    rsm->GetTranslation());

        // (Object) Bounds.
        const Bounds b = rsm->GetBounds();
        EXPECT_PTS_CLOSE(Point3f(0, 0, 0),   b.GetCenter());
        EXPECT_VECS_CLOSE(Vector3f(1, 1, 1), b.GetSize());

        // Undo and redo the revSurf change for second iteration.
        context.command_manager->Undo();
        context.command_manager->Redo();
    }
}

TEST_F(SessionTestBase, RevSurfSessionTest2) {
    // Same as above, but the sweep angle is changed to 120 degrees. The
    // translation must be adjusted to compensate for the apparent change in
    // position.
    LoadSession("RevSurf2.mvr");

    // Do this twice - the second time after undo/redo.
    for (int i = 0; i < 2; ++i) {
        SCOPED_TRACE(i == 0 ? "First Iteration" : "Second Iteration");

        const auto &rm = *context.scene_context->root_model;
        EXPECT_EQ(1U, rm.GetChildModelCount());
        const auto rsm = Util::CastToDerived<RevSurfModel>(rm.GetChildModel(0));
        ASSERT_TRUE(rsm);

        const Rotationf xrot = GetXRot_();

        // Smaller sweep angle means the RevSurfModel is 3/4 size in X and 1/2
        // size in Y. Adjust bounds size and translation appropriately. (Move
        // along -Y and +X in local/stage coordinates.)

        const float s = TK::kRevSurfHeight;
        EXPECT_VECS_CLOSE(Vector3f(s, 2 * s, s), rsm->GetScale());
        EXPECT_ROTS_CLOSE(xrot,                  rsm->GetRotation());
        EXPECT_VECS_CLOSE(Vector3f(5.5f, 3, 0),  rsm->GetTranslation());

        // (Object) Bounds are smaller due to smaller sweep angle.
        const Bounds b = rsm->GetBounds();
        EXPECT_PTS_CLOSE(Point3f(0, 0, 0),        b.GetCenter());
        EXPECT_VECS_CLOSE(Vector3f(.75f, 1, .5f), b.GetSize());

        // Undo and redo the revSurf change for second iteration.
        context.command_manager->Undo();
        context.command_manager->Redo();
    }
}
