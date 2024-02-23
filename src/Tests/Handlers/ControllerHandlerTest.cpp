#include "Base/Event.h"
#include "Handlers/ControllerHandler.h"
#include "Items/Controller.h"
#include "Items/RadialMenu.h"
#include "SG/Search.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ControllerHandlerTest : public SceneTestBase {};

static const Str kContents = R"(
  children: [
    Node {
      TEMPLATES: [
        <"nodes/templates/RadialMenu.emd">,
        <"nodes/templates/Controller.emd">,
      ],
      children: [
        <"nodes/Controllers.emd">,
        CLONE "T_RadialMenu" "LeftMenu"  {},
        CLONE "T_RadialMenu" "RightMenu" {},
      ],
    }
  ]
)";

TEST_F(ControllerHandlerTest, Events) {
    ControllerHandler ch;

    ReadRealScene(kContents);
    auto lc =
        SG::FindTypedNodeInScene<Controller>(*GetScene(), "LeftController");
    auto rc =
        SG::FindTypedNodeInScene<Controller>(*GetScene(), "RightController");
    auto lm = SG::FindTypedNodeInScene<RadialMenu>(*GetScene(), "LeftMenu");
    auto rm = SG::FindTypedNodeInScene<RadialMenu>(*GetScene(), "RightMenu");

    ch.SetControllers(lc, rc);
    ch.SetRadialMenus(lm, rm);

    Event levent, revent;
    EXPECT_ENUM_EQ(Handler::HandleCode::kNotHandled, ch.HandleEvent(levent));
    EXPECT_EQ(Vector3f(0, 0, 0),     lc->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0),     rc->GetTranslation());
    EXPECT_EQ(Rotationf::Identity(), lc->GetRotation());
    EXPECT_EQ(Rotationf::Identity(), rc->GetRotation());

    // Events that update the Controller positions and orientations. Note that
    // the ControllerHandler returns false for these events so other handlers
    // can also deal with them.
    levent.device = Event::Device::kLeftController;
    revent.device = Event::Device::kRightController;
    levent.flags.Set(Event::Flag::kPosition3D);
    revent.flags.Set(Event::Flag::kPosition3D);
    levent.flags.Set(Event::Flag::kOrientation);
    revent.flags.Set(Event::Flag::kOrientation);
    levent.position3D.Set(-1, 0, 0);
    revent.position3D.Set( 1, 0, 0);
    levent.orientation = BuildRotation(0, 0, 0, -10);
    revent.orientation = BuildRotation(0, 0, 0,  10);
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledContinue,
                   ch.HandleEvent(levent));
    EXPECT_EQ(Vector3f(-1, 0, 0),          lc->GetTranslation());
    EXPECT_EQ(Vector3f( 0, 0, 0),          rc->GetTranslation());
    EXPECT_EQ(BuildRotation(0, 0, 0, -10), lc->GetRotation());
    EXPECT_EQ(Rotationf::Identity(),       rc->GetRotation());
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledContinue,
                   ch.HandleEvent(revent));
    EXPECT_EQ(Vector3f(-1, 0, 0),          lc->GetTranslation());
    EXPECT_EQ(Vector3f( 1, 0, 0),          rc->GetTranslation());
    EXPECT_EQ(BuildRotation(0, 0, 0, -10), lc->GetRotation());
    EXPECT_EQ(BuildRotation(0, 0, 0,  10), rc->GetRotation());

    // Events that also update a RadialMenu highlight are ignored if the
    // RadialMenu is enabled. They are disabled by default.
    levent.flags.Set(Event::Flag::kPosition2D);
    revent.flags.Set(Event::Flag::kPosition2D);
    levent.position2D.Set(-.5f, .5f);
    levent.position2D.Set( .5f, .5f);
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledContinue,
                   ch.HandleEvent(levent));
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledContinue,
                   ch.HandleEvent(revent));
    lm->SetEnabled(true);
    rm->SetEnabled(true);
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledContinue,
                   ch.HandleEvent(levent));
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledContinue,
                   ch.HandleEvent(revent));

    // No position should clear the highlight.
    levent.flags.Reset(Event::Flag::kPosition2D);
    revent.flags.Reset(Event::Flag::kPosition2D);
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledContinue,
                   ch.HandleEvent(levent));
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledContinue,
                   ch.HandleEvent(revent));

    // Trackpad buttom press causes the event to be handled.
    levent.flags.Set(Event::Flag::kButtonPress);
    revent.flags.Set(Event::Flag::kButtonPress);
    levent.button = Event::Button::kUp;
    revent.button = Event::Button::kDown;
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, ch.HandleEvent(levent));
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, ch.HandleEvent(revent));
}
