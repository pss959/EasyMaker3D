//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Panes/RadioButtonPane.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class RadioButtonPaneTest : public PaneTestBase {
  protected:
    RadioButtonPanePtr GetRadioButtonPane(const Str &contents = "") {
        return ReadRealPane<RadioButtonPane>("RadioButtonPane", contents);
    }
};

TEST_F(RadioButtonPaneTest, Defaults) {
    auto rbut = GetRadioButtonPane();
    EXPECT_FALSE(rbut->GetState());
    EXPECT_NOT_NULL(rbut->GetActivationWidget());
    EXPECT_EQ(rbut.get(),        rbut->GetInteractor());
    EXPECT_EQ(rbut->GetBorder(), rbut->GetFocusBorder());
}

TEST_F(RadioButtonPaneTest, SetActivateAndNotify) {
    auto rbut = GetRadioButtonPane();

    size_t change_count = 0;
    size_t cur_index    = 1000;
    rbut->GetStateChanged().AddObserver("key", [&](size_t index){
        ++change_count;
        cur_index = index;
    });
    EXPECT_EQ(0U,    change_count);
    EXPECT_EQ(1000U, cur_index);

    rbut->Activate();
    EXPECT_TRUE(rbut->GetState());
    EXPECT_EQ(1U, change_count);
    EXPECT_EQ(0U, cur_index);  // Button is not in a group.

    rbut->Activate();
    EXPECT_FALSE(rbut->GetState());
    EXPECT_EQ(2U, change_count);
    EXPECT_EQ(0U, cur_index);  // Button is not in a group.
}

TEST_F(RadioButtonPaneTest, Group) {
    auto rbut = GetRadioButtonPane();

    auto rbut0 = rbut->CloneTyped<RadioButtonPane>(true);
    auto rbut1 = rbut->CloneTyped<RadioButtonPane>(true);
    auto rbut2 = rbut->CloneTyped<RadioButtonPane>(true);

    size_t cur_index = 1000;
    auto func = [&](size_t index){ cur_index = index; };
    rbut0->GetStateChanged().AddObserver("key", func);
    rbut1->GetStateChanged().AddObserver("key", func);
    rbut2->GetStateChanged().AddObserver("key", func);

    RadioButtonPane::CreateGroup(
        std::vector<RadioButtonPanePtr>{rbut0, rbut1, rbut2}, 1);

    // Index 1 was specified as selected.
    EXPECT_EQ(1U, cur_index);
    EXPECT_FALSE(rbut0->GetState());
    EXPECT_TRUE(rbut1->GetState());
    EXPECT_FALSE(rbut2->GetState());

    rbut2->Activate();
    EXPECT_EQ(2U, cur_index);
    EXPECT_FALSE(rbut0->GetState());
    EXPECT_FALSE(rbut1->GetState());
    EXPECT_TRUE(rbut2->GetState());

    rbut0->Activate();
    EXPECT_EQ(0U, cur_index);
    EXPECT_TRUE(rbut0->GetState());
    EXPECT_FALSE(rbut1->GetState());
    EXPECT_FALSE(rbut2->GetState());
}
