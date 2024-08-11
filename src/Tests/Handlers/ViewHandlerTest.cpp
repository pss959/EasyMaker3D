//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Base/Event.h"
#include "Handlers/ViewHandler.h"
#include "SG/WindowCamera.h"
#include "Tests/TestBaseWithTypes.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ViewHandlerTest : public TestBaseWithTypes {};

TEST_F(ViewHandlerTest, RotationCenter) {
    ViewHandler vh;
    EXPECT_EQ(Point3f::Zero(), vh.GetRotationCenter());
    vh.SetRotationCenter(Point3f(1, 2, 3));
    EXPECT_EQ(Point3f(1, 2, 3), vh.GetRotationCenter());
}

TEST_F(ViewHandlerTest, View) {
    auto cam = CreateObject<SG::WindowCamera>("Camera");

    ViewHandler vh;
    vh.SetCamera(cam);
    vh.SetFixedCameraPosition(false);  // Position will be updated.

    EXPECT_EQ(Rotationf::Identity(), cam->GetOrientation());
    EXPECT_EQ(Point3f(0, 0, -10),    cam->GetPosition());

    Event event;
    EXPECT_ENUM_EQ(Handler::HandleCode::kNotHandled, vh.HandleEvent(event));

    // Event to start rotating the camera.
    event.device = Event::Device::kMouse;
    event.flags.Set(Event::Flag::kButtonPress);
    event.flags.Set(Event::Flag::kPosition2D);
    event.button = Event::Button::kMouse3;
    event.position2D.Set(0, 0);
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, vh.HandleEvent(event));

    // Rotate.
    event.flags.Reset(Event::Flag::kButtonPress);
    event.position2D.Set(1, 0);
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, vh.HandleEvent(event));
    EXPECT_ROTS_CLOSE(BuildRotation(0, 1, 0, 57.2958f), cam->GetOrientation());
    EXPECT_PTS_CLOSE(Point3f(8.41471f, 0, 5.40302f),    cam->GetPosition());

    // Stop rotating.
    event.flags.Set(Event::Flag::kButtonRelease);
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, vh.HandleEvent(event));
    EXPECT_ROTS_CLOSE(BuildRotation(0, 1, 0, 57.2958f), cam->GetOrientation());
    EXPECT_PTS_CLOSE(Point3f(8.41471f, 0, 5.40302f),    cam->GetPosition());

    // Key press to reset the view.
    event.device = Event::Device::kKeyboard;
    event.flags.SetAll(false);
    event.flags.Set(Event::Flag::kKeyPress);
    Str error;
    EXPECT_TRUE(Event::ParseKeyString("Ctrl-.", event.modifiers,
                                      event.key_name, error));
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, vh.HandleEvent(event));
    EXPECT_EQ(Rotationf::Identity(), cam->GetOrientation());
    EXPECT_EQ(Point3f(0, 0, 10),     cam->GetPosition());
}
