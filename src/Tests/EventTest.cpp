#include "Base/Event.h"
#include "Testing.h"

TEST(Event, Defaults) {
    Event ev;
    EXPECT_ENUM_EQ(Event::Device::kUnknown, ev.device);
    EXPECT_FALSE(ev.flags.HasAny());
    EXPECT_ENUM_EQ(Event::Button::kNone, ev.button);
    EXPECT_TRUE(ev.GetKeyString().empty());
    EXPECT_TRUE(ev.GetControllerButtonString().empty());
    EXPECT_EQ(0, ev.position1D);
    EXPECT_EQ(Point2f::Zero(), ev.position2D);
    EXPECT_EQ(Point3f::Zero(), ev.position3D);
    EXPECT_TRUE(ev.orientation.IsIdentity());
}

TEST(Event, GetKeyString) {
    Event ev;
    ev.device = Event::Device::kKeyboard;
    ev.flags.Set(Event::Flag::kKeyPress);

    ev.key_name = "q";
    EXPECT_EQ("q", ev.GetKeyString());
    ev.modifiers.Set(Event::ModifierKey::kControl);
    EXPECT_EQ("<Ctrl>q", ev.GetKeyString());
    ev.modifiers.Set(Event::ModifierKey::kShift);
    EXPECT_EQ("<Shift><Ctrl>q", ev.GetKeyString());
    ev.modifiers.Set(Event::ModifierKey::kAlt);
    EXPECT_EQ("<Shift><Ctrl><Alt>q", ev.GetKeyString());
    ev.modifiers.Reset(Event::ModifierKey::kControl);
    EXPECT_EQ("<Shift><Alt>q", ev.GetKeyString());
}

TEST(Event, GetControllerButtonString) {
    Event ev;

    ev.flags.Set(Event::Flag::kButtonPress);

    ev.device = Event::Device::kLeftController;
    ev.button = Event::Button::kCenter;
    EXPECT_EQ("L:Center", ev.GetControllerButtonString());

    ev.device = Event::Device::kRightController;
    ev.button = Event::Button::kMenu;
    EXPECT_EQ("R:Menu", ev.GetControllerButtonString());
}
