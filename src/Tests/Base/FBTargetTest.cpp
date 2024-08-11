//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Base/FBTarget.h"
#include "Tests/Testing.h"

TEST(FBTargetTest, FBTarget) {
    FBTarget fb_target;

    EXPECT_FALSE(fb_target.IsInitialized());
    EXPECT_NULL(fb_target.GetRenderedFBO().Get());
    EXPECT_NULL(fb_target.GetResolvedFBO().Get());

    fb_target.Init("Testing", Vector2ui(400, 300), 2);
    EXPECT_TRUE(fb_target.IsInitialized());
    EXPECT_NOT_NULL(fb_target.GetRenderedFBO().Get());
    EXPECT_NOT_NULL(fb_target.GetResolvedFBO().Get());
}
