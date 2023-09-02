#include "Place/ClickInfo.h"
#include "Tests/Testing.h"

TEST(ClickInfoTest, ClickInfo) {
    ClickInfo info;
    EXPECT_EQ(Event::Device::kUnknown, info.device);
    EXPECT_FALSE(info.hit.IsValid());
    EXPECT_FALSE(info.is_long_press);
    EXPECT_FALSE(info.is_modified_mode);
    EXPECT_NULL(info.widget);
}

TEST(ClickInfoTest, Equality) {
    const ClickInfo info0;
    const ClickInfo info1;
    ClickInfo       info2;

    info2.is_modified_mode = true;
    EXPECT_EQ(info0, info0);
    EXPECT_EQ(info0, info1);
    EXPECT_NE(info0, info2);
}
