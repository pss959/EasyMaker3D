//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Panes/CheckboxPane.h"
#include "Place/ClickInfo.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"
#include "Widgets/ClickableWidget.h"

/// \ingroup Tests
class CheckboxPaneTest : public PaneTestBase {
  protected:
    CheckboxPanePtr GetCheckboxPane(const Str &contents = "") {
        return ReadRealPane<CheckboxPane>("CheckboxPane", contents);
    }
};

TEST_F(CheckboxPaneTest, Defaults) {
    auto cbox = GetCheckboxPane();
    EXPECT_FALSE(cbox->GetState());
    EXPECT_EQ(cbox.get(),        cbox->GetInteractor());
    EXPECT_EQ(cbox->GetBorder(), cbox->GetFocusBorder());
}

TEST_F(CheckboxPaneTest, SetAndNotify) {
    auto cbox = GetCheckboxPane();

    size_t change_count = 0;
    cbox->GetStateChanged().AddObserver("key", [&](){ ++change_count; });
    EXPECT_EQ(0U, change_count);

    ClickInfo info;  // Contents do not matter.

    cbox->GetActivationWidget()->Click(info);
    EXPECT_TRUE(cbox->GetState());
    EXPECT_EQ(1U, change_count);

    cbox->GetActivationWidget()->Click(info);
    EXPECT_FALSE(cbox->GetState());
    EXPECT_EQ(2U, change_count);
}
