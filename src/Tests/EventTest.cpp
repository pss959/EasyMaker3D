#include "Event.h"
#include "Testing.h"

TEST(Event, Defaults) {
    Event ev;
    EXPECT_EQ(Event::Device::kUnknown, ev.device);
    //CheckEnumsEqual(Event::Device::kUnknown, ev.device);
    EXPECT_FALSE(ev.flags.HasAny());
    //CheckEnumsEqual(Event::Button::kNone, ev.button);
    EXPECT_EQ(Event::Button::kNone, ev.button);
    EXPECT_TRUE(ev.GetKeyString().empty());
    EXPECT_EQ(0, ev.position1D);
    EXPECT_EQ(Point2f::Zero(), ev.position2D);
    EXPECT_EQ(Point3f::Zero(), ev.position3D);
    EXPECT_TRUE(ev.orientation.IsIdentity());
}
