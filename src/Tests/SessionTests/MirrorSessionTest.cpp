#include "App/SceneContext.h"
#include "Base/Tuning.h"
#include "Math/Types.h"
#include "Models/MirroredModel.h"
#include "Models/RootModel.h"
#include "SessionTests/SessionTestBase.h"
#include "Testing.h"
#include "Util/General.h"

TEST_F(SessionTestBase, MirrorSessionTest) {
    // This has 3 cylinders converted to MirroredModels:
    //  - Cylinder_1 / Mirrored_1 is untransformed.
    //  - Cylinder_2 is rotated 90 around X and translated by 5 in X
    //      BEFORE converting to Mirrored_2.
    //  - Cylinder_3 is translated by 10 in X BEFORE converting to Mirrored_3
    //      and rotated 90 around X AFTER converting.
    LoadSession("Mirror.mvr");

    const auto &rm = *context.scene_context->root_model;
    EXPECT_EQ(3U, rm.GetChildModelCount());
    const auto mm1 = Util::CastToDerived<MirroredModel>(rm.GetChildModel(0));
    const auto mm2 = Util::CastToDerived<MirroredModel>(rm.GetChildModel(1));
    const auto mm3 = Util::CastToDerived<MirroredModel>(rm.GetChildModel(2));
    ASSERT_TRUE(mm1);
    ASSERT_TRUE(mm2);
    ASSERT_TRUE(mm3);
    const auto or1 = mm1->GetOriginalModel();
    const auto or2 = mm2->GetOriginalModel();
    const auto or3 = mm3->GetOriginalModel();

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
    EXPECT_EQ(1U, mm1->GetPlaneNormals().size());
    EXPECT_EQ(1U, mm2->GetPlaneNormals().size());
    EXPECT_EQ(1U, mm3->GetPlaneNormals().size());
    EXPECT_VECS_CLOSE(Vector3f::AxisX(), mm1->GetPlaneNormals()[0]);
    EXPECT_VECS_CLOSE(Vector3f::AxisX(), mm2->GetPlaneNormals()[0]);
    EXPECT_VECS_CLOSE(Vector3f::AxisX(), mm3->GetPlaneNormals()[0]);

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

TEST_F(SessionTestBase, MirrorInPlaceSessionTest) {
    // Same as above, but mirroring in place, so no change in translation.
    LoadSession("MirrorInPlace.mvr");

    const auto &rm = *context.scene_context->root_model;
    EXPECT_EQ(3U, rm.GetChildModelCount());
    const auto mm1 = Util::CastToDerived<MirroredModel>(rm.GetChildModel(0));
    const auto mm2 = Util::CastToDerived<MirroredModel>(rm.GetChildModel(1));
    const auto mm3 = Util::CastToDerived<MirroredModel>(rm.GetChildModel(2));
    ASSERT_TRUE(mm1);
    ASSERT_TRUE(mm2);
    ASSERT_TRUE(mm3);
    const auto or1 = mm1->GetOriginalModel();
    const auto or2 = mm2->GetOriginalModel();
    const auto or3 = mm3->GetOriginalModel();

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

    // Mirrored models transforms. Translations should not change.
    EXPECT_VECS_CLOSE(Vector3f(s, s, s),     mm1->GetScale());
    EXPECT_VECS_CLOSE(Vector3f(s, s, s),     mm2->GetScale());
    EXPECT_VECS_CLOSE(Vector3f(s, s, s),     mm3->GetScale());
    EXPECT_ROTS_CLOSE(Rotationf::Identity(), mm1->GetRotation());
    EXPECT_ROTS_CLOSE(xrot,                  mm2->GetRotation());
    EXPECT_ROTS_CLOSE(xrot,                  mm3->GetRotation());
    EXPECT_VECS_CLOSE(Vector3f(0,  2, 0),    mm1->GetTranslation());
    EXPECT_VECS_CLOSE(Vector3f(5,  2, 0),    mm2->GetTranslation());
    EXPECT_VECS_CLOSE(Vector3f(10, 2, 0),    mm3->GetTranslation());

    // Mirror planes in object coordinates,
    EXPECT_EQ(1U, mm1->GetPlaneNormals().size());
    EXPECT_EQ(1U, mm2->GetPlaneNormals().size());
    EXPECT_EQ(1U, mm3->GetPlaneNormals().size());
    EXPECT_VECS_CLOSE(Vector3f::AxisX(), mm1->GetPlaneNormals()[0]);
    EXPECT_VECS_CLOSE(Vector3f::AxisX(), mm2->GetPlaneNormals()[0]);
    EXPECT_VECS_CLOSE(Vector3f::AxisX(), mm3->GetPlaneNormals()[0]);

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
