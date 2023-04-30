#include "Managers/CommandManager.h"
#include "Managers/SceneContext.h"
#include "Math/Linear.h"
#include "Math/Types.h"
#include "Models/ClippedModel.h"
#include "Models/RootModel.h"
#include "SessionTests/SessionTestBase.h"
#include "Tests/Testing.h"
#include "Util/General.h"
#include "Util/String.h"
#include "Util/Tuning.h"

// ----------------------------------------------------------------------------
// ClipSessionTest base class - provides helpers for tests.
// ----------------------------------------------------------------------------

class ClipSessionTest : public SessionTestBase {
  protected:
    struct ModelData {
        // Transformation data.
        Vector3f  scale;
        Rotationf rot;
        Vector3f  op_trans;
        Vector3f  clipped_trans;
        // Clip plane.
        Plane     plane;
        // Bounds.
        Bounds    clipped_bounds;
        // Centering offset;
        Vector3f  center_offset;
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
    auto &operand = *cm.GetOperandModel();

    // Select the operand model to make sure it is up to date.
    SelectModel(operand);

    // Operand model transform.
    EXPECT_VECS_CLOSE(expected.scale,    operand.GetScale());
    EXPECT_ROTS_CLOSE(expected.rot,      operand.GetRotation());
    EXPECT_VECS_CLOSE(expected.op_trans, operand.GetTranslation());

    SelectModel(cm);

    // ClippedModel transform.
    EXPECT_VECS_CLOSE(expected.scale,         cm.GetScale());
    EXPECT_ROTS_CLOSE(expected.rot,           cm.GetRotation());
    EXPECT_VECS_CLOSE(expected.clipped_trans, cm.GetTranslation());

    // Clipping plane.
    EXPECT_CLOSE(expected.plane.distance,    cm.GetPlane().distance);
    EXPECT_VECS_CLOSE(expected.plane.normal, cm.GetPlane().normal);

    // Bounds.
    const Bounds cb = cm.GetBounds();
    EXPECT_PTS_CLOSE(expected.clipped_bounds.GetCenter(), cb.GetCenter());
    EXPECT_VECS_CLOSE(expected.clipped_bounds.GetSize(),  cb.GetSize());

    // ClippedModel mesh offset (in local coordinates).
    EXPECT_VECS_CLOSE(expected.center_offset, cm.GetLocalCenterOffset());
}

void ClipSessionTest::CompareN(const std::vector<ModelData> &expected) {
    const auto &rm = *context.scene_context->root_model;
    EXPECT_EQ(expected.size(), rm.GetChildModelCount());

    for (size_t i = 0; i < expected.size(); ++i) {
        const auto cm = Util::CastToDerived<ClippedModel>(rm.GetChildModel(i));
        ASSERT_TRUE(cm);
        SCOPED_TRACE("Model " + cm->GetName() + " / " +
                     cm->GetOperandModel()->GetName());
        Compare1(expected[i], *cm);
    }
}

// ----------------------------------------------------------------------------
// Tests.
// ----------------------------------------------------------------------------

TEST_F(ClipSessionTest, ClipSessionTest0) {
    // This has 1 cylinder (Cylinder_1) that has a default transform, then
    // converted to a ClippedModel (Clipped_1) with the default clipping plane.
    //
    // Clipped_1 should have its top half clipped away and sit on the Y=0
    // plane.
    LoadSession("Clip0");

    const float s = TK::kInitialModelScale;

    ModelData md;
    md.scale.Set(s, s, s);
    md.rot = Rotationf::Identity();
    md.op_trans.Set(0, 2, 0);
    md.clipped_trans.Set(0, 1, 0);
    md.plane = ClippedModel::GetDefaultPlane();
    md.clipped_bounds = Bounds(Vector3f(2, 1, 2));
    md.center_offset.Set(0, -1, 0);

    // Do this twice - the second time after undo/redo.
    for (int i = 0; i < 2; ++i) {
        SCOPED_TRACE("Iteration " + Util::ToString(i));

        CompareN(std::vector<ModelData>(1, md));

        // Undo and redo the clip change for second iteration.
        context.command_manager->Undo();
        context.command_manager->Redo();
    }
}

TEST_F(ClipSessionTest, ClipSessionTest1) {
    // This has 1 cylinder (Cylinder_1) that has its height scaled by 2,
    // rotated by 90 degrees around X, and translated by 5 in X, then converted
    // to a ClippedModel (Clipped_1). Clipped_1 is then clipped by the Z=0
    // plane in stage coordinates, which is the Y=0 plane in object
    // coordinates.
    //
    // Clipped_1 should have its rotated top half (front) clipped away.
    LoadSession("Clip1");

    const float s = TK::kInitialModelScale;

    ModelData md;
    md.scale.Set(s, 2 * s, s);
    md.rot = GetXRot();
    md.op_trans.Set(5, 4, 0);
    md.clipped_trans.Set(5, 4, -2);
    md.plane = Plane(0, Vector3f::AxisY());
    md.clipped_bounds = Bounds(Vector3f(2, 1, 2));
    md.center_offset.Set(0, 0, -2);

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
    // This has 1 cylinder (Cylinder_1) that is scaled by 2 in height and then
    // converted to a ClippedModel (Clipped_1). It should have its front half
    // clipped away.
    LoadSession("Clip1b");

    const float s = TK::kInitialModelScale;

    ModelData md;
    md.scale.Set(s, 2 * s, s);
    md.rot = Rotationf::Identity();
    md.op_trans.Set(0, 4, 0);
    md.clipped_trans.Set(0,  4, -1);
    md.plane = Plane(0, Vector3f::AxisZ());
    md.clipped_bounds = Bounds(Vector3f(2, 2, 1));
    md.center_offset.Set(0, 0, -1);

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

    LoadSession("Clip3");

    const float s = TK::kInitialModelScale;

    ModelData md1, md2, md3;
    md1.scale.Set(s, 2 * s, s);
    md2.scale.Set(s, 2 * s, s);
    md3.scale.Set(s, 2 * s, s);
    md1.rot = Rotationf::Identity();
    md2.rot = GetXRot();
    md3.rot = GetXRot();
    md1.op_trans.Set(0,  4, 0);
    md2.op_trans.Set(5,  5, -2);
    md3.op_trans.Set(10, 2, 2);
    md1.clipped_trans.Set(0,  4, -1);
    md2.clipped_trans.Set(5,  5, -3);
    md3.clipped_trans.Set(10, 2, -1);
    // Clipping planes. The clip plane is the Z=0 plane in stage coordinates,
    // with the normal pointing along +Z. The planes in the ClippedModels are
    // in object coordinates.
    md1.plane = Plane(0,    Vector3f::AxisZ());
    md2.plane = Plane(.5f,  Vector3f::AxisY());
    md3.plane = Plane(-.5f, Vector3f::AxisY());
    md1.clipped_bounds = Bounds(Vector3f(2, 2, 1));
    md2.clipped_bounds = Bounds(Vector3f(2, 1.5f, 2));
    md3.clipped_bounds = Bounds(Vector3f(2, .5f, 2));
    md1.center_offset.Set(0, 0, -1);
    md2.center_offset.Set(0, 0, -1);
    md3.center_offset.Set(0, 0, -3);

    // Do this twice - the second time after undo/redo.
    for (int i = 0; i < 2; ++i) {
        SCOPED_TRACE("Iteration " + Util::ToString(i));

        CompareN(std::vector<ModelData>{md1, md2, md3});

        // Undo and redo the clip change for second iteration.
        context.command_manager->Undo();
        context.command_manager->Redo();
    }
}
