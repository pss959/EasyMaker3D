//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Enums/Action.h"
#include "Tests/Testing.h"
#include "Util/Enum.h"

TEST(ActionTest, IsToggleAction) {
    for (const auto action: Util::EnumValues<Action>()) {
        if (Util::EnumName(action).starts_with("kToggle")) {
            EXPECT_TRUE(IsToggleAction(action));
        }
        else {
            EXPECT_FALSE(IsToggleAction(action));
        }
    }
}
