#include "SG/WindowCamera.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Viewers/GLFWViewer.h"

/// \ingroup Tests
class GLFWViewerTest : public SceneTestBase {};

TEST_F(GLFWViewerTest, Camera) {
    auto cam   = ParseTypedObject<SG::WindowCamera>(R"(
    WindowCamera {
      position: 1 2 3,
      orientation: 0 1 0 30,
      fov: 45,
      near: 2,
      far: 10,
    })");

    GLFWViewer viewer;
    viewer.Init(Vector2i(600, 600), false);  // Square aspect ratio.
    EXPECT_EQ(Vector2i(600, 600), viewer.GetWindowSize());

    // Camera->Frustum.
    viewer.SetCamera(cam);
    EXPECT_NOT_NULL(viewer.GetFrustum());
    const auto &fr = *viewer.GetFrustum();
    const Anglef a = Anglef::FromDegrees(22.5f);
    const auto rot = BuildRotation(0, 1, 0, 30);
    EXPECT_EQ(Vector2i(600, 600), fr.viewport.GetSize());
    EXPECT_EQ(Point3f(1, 2, 3),   fr.position);
    EXPECT_EQ(rot,                fr.orientation);
    EXPECT_EQ(-a,                 fr.fov_left);
    EXPECT_EQ( a,                 fr.fov_right);
    EXPECT_EQ(-a,                 fr.fov_down);
    EXPECT_EQ( a,                 fr.fov_up);
    EXPECT_EQ(2,                  fr.pnear);
    EXPECT_EQ(10,                 fr.pfar);
}

TEST_F(GLFWViewerTest, Events) {
    // Call trivial stuff.
    GLFWViewer viewer;
    viewer.SetPollEventsFlag(true);
    viewer.EnableMouseMotionEvents(true);
    EXPECT_FALSE(viewer.IsShiftKeyPressed());
}
