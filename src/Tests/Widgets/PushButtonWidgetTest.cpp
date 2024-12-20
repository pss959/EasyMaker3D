//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Place/ClickInfo.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Widgets/PushButtonWidget.h"

/// \ingroup Tests
class PushButtonWidgetTest : public SceneTestBase {};

TEST_F(PushButtonWidgetTest, Defaults) {
    auto pbw = CreateObject<PushButtonWidget>();

    EXPECT_FALSE(pbw->IsToggle());
    EXPECT_FALSE(pbw->GetToggleState());
}

TEST_F(PushButtonWidgetTest, Toggle) {
    auto pbw = CreateObject<PushButtonWidget>();

    pbw->SetIsToggle(true);
    EXPECT_TRUE(pbw->IsToggle());
    EXPECT_FALSE(pbw->GetToggleState());
    pbw->SetToggleState(true);
    EXPECT_TRUE(pbw->IsToggle());
    EXPECT_TRUE(pbw->GetToggleState());
    pbw->SetToggleState(false);
    EXPECT_TRUE(pbw->IsToggle());
    EXPECT_FALSE(pbw->GetToggleState());
    pbw->SetToggleState(true);
    pbw->SetIsToggle(false);
    EXPECT_FALSE(pbw->IsToggle());
    EXPECT_FALSE(pbw->GetToggleState());
}

TEST_F(PushButtonWidgetTest, Click) {
    auto pbw = CreateObject<PushButtonWidget>();

    ClickInfo info;
    info.device = Event::Device::kMouse;
    info.hit.point = Point3f(1, 2, 3);
    info.is_long_press = true;
    info.is_modified_mode = true;
    info.widget = pbw.get();

    size_t click_count = 0;

    pbw->GetClicked().AddObserver("key", [&](const ClickInfo &ci){
        EXPECT_EQ(info, ci);
        ++click_count;
    });

    EXPECT_EQ(0U, click_count);
    pbw->Click(info);
    EXPECT_EQ(1U, click_count);
    pbw->Click(info);
    EXPECT_EQ(2U, click_count);
}

TEST_F(PushButtonWidgetTest, ClickToggle) {
    auto pbw = CreateObject<PushButtonWidget>();
    pbw->SetIsToggle(true);

    ClickInfo info;
    info.device = Event::Device::kMouse;
    info.widget = pbw.get();

    size_t click_count = 0;

    pbw->GetClicked().AddObserver("key", [&](const ClickInfo &ci){
        EXPECT_EQ(info, ci);
        ++click_count;
    });

    EXPECT_EQ(0U, click_count);
    EXPECT_FALSE(pbw->GetToggleState());
    pbw->Click(info);
    EXPECT_EQ(1U, click_count);
    EXPECT_TRUE(pbw->GetToggleState());
    pbw->Click(info);
    EXPECT_EQ(2U, click_count);
    EXPECT_FALSE(pbw->GetToggleState());
    pbw->Click(info);
    EXPECT_EQ(3U, click_count);
    EXPECT_TRUE(pbw->GetToggleState());
}
