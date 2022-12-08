#include "App/SceneContext.h"
#include "Base/Tuning.h"
#include "Managers/CommandManager.h"
#include "Math/Linear.h"
#include "Math/Types.h"
#include "Models/ClippedModel.h"
#include "Models/RootModel.h"
#include "SessionTests/SessionTestBase.h"
#include "Tests/Testing.h"
#include "Util/General.h"
#include "Util/String.h"

// ----------------------------------------------------------------------------
// ClipSessionTest base class - provides helpers for tests.
// ----------------------------------------------------------------------------

class ClipSessionTest : public SessionTestBase {
  protected:
    struct ModelData {
        // Transformation data.
        Vector3f           scale;
        Rotationf          rot;
        Vector3f           orig_trans;
        Vector3f           clipped_trans;
        // Clip plane data.
        std::vector<Plane> planes;
        // Bounds.
        Bounds             clipped_bounds;
        // Mesh offset;
        Vector3f           mesh_offset;
    };

    void Compare1(const ModelData &expected, ClippedModel &cm);
    void CompareN(const std::vector<ModelData> &expected);

    // Convenience function
    static Rotationf GetXRot() {
        return Rotationf::FromAxisAndAngle(Vector3f::AxisX(),
                                           Anglef::FromDegrees(90));
    }
};

void ClipSessionTest::Compare1(const ModelData &expected, ClippedModel &cm) {
    auto &orig = *cm.GetOriginalModel();

    // Make sure model transformations are up to date.
    cm.SetStatus(Model::Status::kDescendantShown);
    orig.SetStatus(Model::Status::kPrimary);
    orig.SetStatus(Model::Status::kAncestorShown);
    cm.SetStatus(Model::Status::kPrimary);

    // Original model transform.
    EXPECT_VECS_CLOSE(expected.scale,      orig.GetScale());
    EXPECT_ROTS_CLOSE(expected.rot,        orig.GetRotation());
    EXPECT_VECS_CLOSE(expected.orig_trans, orig.GetTranslation());

    // ClippedModel transform.
    EXPECT_VECS_CLOSE(expected.scale,         cm.GetScale());
    EXPECT_ROTS_CLOSE(expected.rot,           cm.GetRotation());
    EXPECT_VECS_CLOSE(expected.clipped_trans, cm.GetTranslation());

    // Clipping planes.
    EXPECT_EQ(expected.planes.size(), cm.GetPlanes().size());
    for (size_t i = 0; i < expected.planes.size(); ++i) {
        SCOPED_TRACE("Plane " + Util::ToString(i));
        const Plane &ep = expected.planes[i];
        const Plane &cp = cm.GetPlanes()[i];
        EXPECT_NEAR(ep.distance, cp.distance, kClose);
        EXPECT_VECS_CLOSE(ep.normal, cp.normal);
    }

    // Bounds.
    const Bounds cb = cm.GetBounds();
    EXPECT_PTS_CLOSE(expected.clipped_bounds.GetCenter(), cb.GetCenter());
    EXPECT_VECS_CLOSE(expected.clipped_bounds.GetSize(),  cb.GetSize());

    // ClippedModel mesh offset (in object coordinates).
    EXPECT_VECS_CLOSE(expected.mesh_offset, cm.GetMeshOffset());
}

void ClipSessionTest::CompareN(const std::vector<ModelData> &expected) {
    const auto &rm = *context.scene_context->root_model;
    EXPECT_EQ(expected.size(), rm.GetChildModelCount());

    for (size_t i = 0; i < expected.size(); ++i) {
        const auto cm = Util::CastToDerived<ClippedModel>(rm.GetChildModel(i));
        ASSERT_TRUE(cm);
        SCOPED_TRACE("Model " + cm->GetName() + " / " +
                     cm->GetOriginalModel()->GetName());
        Compare1(expected[i], *cm);
    }
}

// ----------------------------------------------------------------------------
// Tests.
// ----------------------------------------------------------------------------

TEST_F(ClipSessionTest, ClipSessionTest1) {
    // This has 1 cylinder (Cylinder_1) that is scaled by 2 in height, rotated
    // by 90 degrees around X, and translated by 5 in X, then converted to a
    // ClippedModel (Clipped_1). Clipped_1 is then clipped by the Z=0 plane in
    // stage coordinates.
    //
    // Clipped_1 should have its rotated top half (front) clipped away.
    LoadSession("Clip1.mvr");

    const float s = TK::kInitialModelScale;

    ModelData md;
    md.scale.Set(s, 2 * s, s);
    md.rot = GetXRot();
    md.orig_trans.Set(5, 4, 0);
    md.clipped_trans.Set(5, 4, -2);
    md.planes.push_back(Plane(0, Vector3f::AxisY()));
    md.clipped_bounds = Bounds(Vector3f(2, 1, 2));
    md.mesh_offset.Set(0, .5f, 0);

    // Do this twice - the second time after undo/redo.
    for (int i = 0; i < 2; ++i) {
        SCOPED_TRACE("Iteration " + Util::ToString(i));

        CompareN(std::vector<ModelData>(1, md));

        // Undo and redo the clip change for second iteration.
        context.command_manager->Undo();
        context.command_manager->Redo();
    }
}

TEST_F(ClipSessionTest, ClipSessionTest1b) {
    // This has 1 cylinder (Cylinder_1) that is scaled by 2 in
    // height and then converted to a ClippedModel (Clipped_1). It should have
    // its front half clipped away.
    LoadSession("Clip1b.mvr");

    const float s = TK::kInitialModelScale;

    ModelData md;
    md.scale.Set(s, 2 * s, s);
    md.rot = Rotationf::Identity();
    md.orig_trans.Set(0, 4, 0);
    md.clipped_trans.Set(0,  4, -1);
    // Clipping planes. The clip plane is the Z=0 plane in stage coordinates,
    // with the normal pointing along +Z. The planes in the ClippedModels are
    // in object coordinates.
    md.planes.push_back(Plane(0, Vector3f::AxisZ()));
    md.clipped_bounds = Bounds(Vector3f(2, 2, 1));
    md.mesh_offset.Set(0, 0, .5f);

    // Do this twice - the second time after undo/redo.
    for (int i = 0; i < 2; ++i) {
        SCOPED_TRACE("Iteration " + Util::ToString(i));

        CompareN(std::vector<ModelData>(1, md));

        // Undo and redo the clip change for second iteration.
        context.command_manager->Undo();
        context.command_manager->Redo();
    }
}

TEST_F(ClipSessionTest, ClipSessionTest3) {
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
    //  - All 3 ClippedModels are clipped by the Z=0 plane (in stage
    //    coordinates).
    //
    // Therefore:
    //   - Clipped_1 should have its front half clipped away.
    //   - Clipped_2 should have its rotated top half clipped away.
    //   - Clipped_3 should have its rotated top half clipped away.
    //
    // All resulting ClippedModel remnants should have the same position as
    // before clipping.

    LoadSession("Clip3.mvr");

    const float s = TK::kInitialModelScale;

    ModelData md1, md2, md3;
    md1.scale.Set(s, 2 * s, s);
    md2.scale.Set(s, 2 * s, s);
    md3.scale.Set(s, 2 * s, s);
    md1.rot = Rotationf::Identity();
    md2.rot = GetXRot();
    md3.rot = GetXRot();
    md1.orig_trans.Set(0, 4, 0);
    md2.orig_trans.Set(5, 4, 0);
    md3.orig_trans.Set(10, 4, 0);
    md1.clipped_trans.Set(0,  4, -1);
    md2.clipped_trans.Set(5,  4, -2);
    md3.clipped_trans.Set(10, 4, -2);
    // Clipping planes. The clip plane is the Z=0 plane in stage coordinates,
    // with the normal pointing along +Z. The planes in the ClippedModels are
    // in object coordinates.
    md1.planes.push_back(Plane(0, Vector3f::AxisZ()));
    md2.planes.push_back(Plane(0, Vector3f::AxisY()));
    md3.planes.push_back(Plane(0, Vector3f::AxisY()));
    md1.clipped_bounds = Bounds(Vector3f(2, 2, 1));
    md2.clipped_bounds = Bounds(Vector3f(2, 1, 2));
    md3.clipped_bounds = Bounds(Vector3f(2, 1, 2));
    md1.mesh_offset.Set(0, 0, .5f);
    md2.mesh_offset.Set(0, .5f, 0);
    md3.mesh_offset.Set(0, .5f, 0);

    // Do this twice - the second time after undo/redo.
    for (int i = 0; i < 2; ++i) {
        SCOPED_TRACE("Iteration " + Util::ToString(i));

        CompareN(std::vector<ModelData>{md1, md2, md3});

        // Undo and redo the clip change for second iteration.
        context.command_manager->Undo();
        context.command_manager->Redo();
    }
}

TEST_F(ClipSessionTest, ClipSessionTest2Planes) {
    // This has 1 cylinder (Cylinder_1) converted to a ClippedModel
    // (Clipped_1). The cylinder is not transformed, so it should have the
    // default size of 4x4x4.
    //
    // There are two planes applied. The first should clip off the top half of
    // the cylinder and the second should clip off the front half of the
    // remaining piece. The resulting ClippedModel should be 4x2x2 and should
    // be centered on (0,1,-1) so that the front is at the Z=0 plane.
    LoadSession("Clip2Planes.mvr");

    const float s = TK::kInitialModelScale;

    ModelData md;
    md.scale.Set(s, s, s);
    md.rot = Rotationf::Identity();
    md.orig_trans.Set(0, 2, 0);
    md.clipped_trans.Set(0, 1, -1);

    // Clipping planes. In stage coordinates, the two planes should be:
    //   1) Y=2 with the normal pointing along +Y.
    //   2) Z=0 with the normal pointing along +Z.
    // In object coordinates of the ClippedModel, the two planes should be:
    //   1) Y=0 with the normal pointing along +Y.
    //   2) Z=0 with the normal pointing along +Z.
    md.planes.push_back(Plane(0, Vector3f::AxisY()));
    md.planes.push_back(Plane(0, Vector3f::AxisZ()));

    md.clipped_bounds = Bounds(Vector3f(2, 1, 1));
    md.mesh_offset.Set(0, .5f, .5f);

    // Do this twice - the second time after undo/redo.
    for (int i = 0; i < 2; ++i) {
        SCOPED_TRACE("Iteration " + Util::ToString(i));
        {
            SCOPED_TRACE("2 Planes");
            CompareN(std::vector<ModelData>(1, md));
        }
        {
            SCOPED_TRACE("1 Plane");
            // Undo and test the one plane case: just the top half should be
            // clipped away.
            context.command_manager->Undo();
            md.planes.pop_back();
            md.clipped_trans.Set(0, 1, 0);
            md.clipped_bounds = Bounds(Vector3f(2, 1, 2));
            md.mesh_offset.Set(0, .5f, 0);

            CompareN(std::vector<ModelData>(1, md));
        }

        // Undo and redo the clip changes for second iteration.
        context.command_manager->Undo();
        context.command_manager->Redo();
        context.command_manager->Redo();

        // Set up for both planes again.
        md.planes.push_back(Plane(0, Vector3f::AxisZ()));
        md.clipped_trans.Set(0, 1, -1);
        md.clipped_bounds = Bounds(Vector3f(2, 1, 1));
        md.mesh_offset.Set(0, .5f, .5f);
    }
}

TEST_F(ClipSessionTest, ClipSessionTest2PlanesScaled) {
    // Same as the above test, but the cylinder is scaled by (1,1.5,2) to the
    // size 4x6x8 and translated by +5 in Z.
    //
    // There are two planes applied. The first should clip off the top half of
    // the cylinder and the second should clip off the front half of the
    // remaining piece. The resulting ClippedModel should be 4x3x4 and should
    // be centered on (0,1.5,3) so that the front is at the Z=5 plane.
    LoadSession("Clip2PlanesScaled.mvr");

    const float s = TK::kInitialModelScale;

    ModelData md;
    md.scale.Set(s, 1.5f * s, 2 * s);
    md.rot = Rotationf::Identity();
    md.orig_trans.Set(0, 3, 5);
    md.clipped_trans.Set(0, 1.5f, 3);

    // Clipping planes. In stage coordinates, the two planes should be:
    //   1) Y=3 with the normal pointing along +Y.
    //   2) Z=0 with the normal pointing along +Z.
    // In object coordinates of the ClippedModel, the two planes should be:
    //   1) Y=0 with the normal pointing along +Y.
    //   2) Z=0 with the normal pointing along +Z.
    md.planes.push_back(Plane(0, Vector3f::AxisY()));
    md.planes.push_back(Plane(0, Vector3f::AxisZ()));

    md.clipped_bounds = Bounds(Vector3f(2, 1, 1));
    md.mesh_offset.Set(0, .5f, .5f);

    // Do this twice - the second time after undo/redo.
    for (int i = 0; i < 2; ++i) {
        SCOPED_TRACE("Iteration " + Util::ToString(i));

        CompareN(std::vector<ModelData>(1, md));

        // Undo and redo both clip changes for second iteration.
        context.command_manager->Undo();
        context.command_manager->Undo();
        context.command_manager->Redo();
        context.command_manager->Redo();
    }
}
