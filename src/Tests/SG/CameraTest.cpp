//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/Frustum.h"
#include "SG/Gantry.h"
#include "SG/VRCamera.h"
#include "SG/WindowCamera.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class CameraTest : public SceneTestBase {};

TEST_F(CameraTest, DefaultGantry) {
    auto gantry = CreateObject<SG::Gantry>();

    EXPECT_NOT_NULL(gantry);
    EXPECT_TRUE(gantry->GetCameras().empty());
    EXPECT_EQ(0, gantry->GetHeight());
}

TEST_F(CameraTest, Cameras) {
    const Str input = R"(
Gantry {
  cameras: [
    WindowCamera {
      position: 1 2 3,
      orientation: 0 1 0 30,
      fov: 45,
      near: 2,
      far: 10,
    },
    VRCamera {
      base_position: 1 2 3,
    },
  ],
},
)";

    auto gantry = ParseTypedObject<SG::Gantry>(input);
    EXPECT_NOT_NULL(gantry);

    EXPECT_EQ(2U, gantry->GetCameras().size());

    auto cam0 = gantry->GetCameras()[0];
    auto cam1 = gantry->GetCameras()[1];
    EXPECT_EQ("WindowCamera", cam0->GetTypeName());
    EXPECT_EQ("VRCamera",     cam1->GetTypeName());
    auto &wcam = *std::dynamic_pointer_cast<SG::WindowCamera>(cam0);
    auto &vcam = *std::dynamic_pointer_cast<SG::VRCamera>(cam1);

    const auto rot0 = BuildRotation(0, 1, 0, 30);
    EXPECT_EQ(Point3f(1, 2, 3),          wcam.GetPosition());
    EXPECT_EQ(rot0,                      wcam.GetOrientation());
    EXPECT_EQ(Anglef::FromDegrees(45),   wcam.GetFOV());
    EXPECT_EQ(2,                         wcam.GetNear());
    EXPECT_EQ(10,                        wcam.GetFar());
    EXPECT_EQ(rot0 * -Vector3f::AxisZ(), wcam.GetViewDirection());
    EXPECT_EQ(Point3f(1, 2, 3),          wcam.GetCurrentPosition());

    EXPECT_EQ(Point3f(1, 2, 3),           vcam.GetBasePosition());
    EXPECT_EQ(Point3f(1, 2, 3),           vcam.GetCurrentPosition());

    EXPECT_EQ(0, gantry->GetHeight());
    EXPECT_EQ(0, wcam.GetHeight());
    EXPECT_EQ(0, vcam.GetHeight());

    // Build a frustum from the WindowCamera.
    Frustum fr;
    wcam.BuildFrustum(Vector2ui(100, 100), fr);
    const Anglef a = Anglef::FromDegrees(22.5f);
    EXPECT_EQ(Vector2ui(100, 100), fr.viewport.GetSize());
    EXPECT_EQ(Point3f(1, 2, 3),    fr.position);
    EXPECT_EQ(rot0,                fr.orientation);
    EXPECT_EQ(-a,                  fr.fov_left);
    EXPECT_EQ( a,                  fr.fov_right);
    EXPECT_EQ(-a,                  fr.fov_down);
    EXPECT_EQ( a,                  fr.fov_up);
    EXPECT_EQ(2,                   fr.pnear);
    EXPECT_EQ(10,                  fr.pfar);

    // Setting the gantry height should update the cameras.
    gantry->SetHeight(10);
    EXPECT_EQ(10, gantry->GetHeight());
    EXPECT_EQ(10, vcam.GetHeight());
    EXPECT_EQ(10, wcam.GetHeight());
    EXPECT_EQ(Point3f(1, 12, 3), vcam.GetCurrentPosition());  // Height added.

    // Modify the cameras and test the results.
    const auto rot1 = BuildRotation(1, 0, 0, 45);
    wcam.SetPosition(Point3f(3, 4, 5));
    wcam.SetOrientation(rot1);
    wcam.SetFOV(Anglef::FromDegrees(80));
    wcam.SetNearAndFar(4, 20);
    EXPECT_EQ(Point3f(3, 4, 5),          wcam.GetPosition());
    EXPECT_EQ(rot1,                      wcam.GetOrientation());
    EXPECT_EQ(Anglef::FromDegrees(80),   wcam.GetFOV());
    EXPECT_EQ(4,                         wcam.GetNear());
    EXPECT_EQ(20,                        wcam.GetFar());
    EXPECT_EQ(rot1 * -Vector3f::AxisZ(), wcam.GetViewDirection());
    EXPECT_EQ(Point3f(3, 4, 5),          wcam.GetCurrentPosition());
}

