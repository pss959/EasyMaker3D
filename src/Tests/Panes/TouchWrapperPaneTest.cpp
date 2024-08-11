//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Panes/TouchWrapperPane.h"
#include "Place/TouchInfo.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"
#include "Util/Tuning.h"
#include "Widgets/Widget.h"

/// \ingroup Tests
class TouchWrapperPaneTest : public PaneTestBase {
  protected:
    // TouchWrapperPane has no template, so this just creates an instance.
    TouchWrapperPanePtr GetTouchWrapperPane(const Str &contents = "") {
        return ReadRealNode<TouchWrapperPane>(contents, "TouchWrapperPane");
    }
};

TEST_F(TouchWrapperPaneTest, Touch) {
    // TouchWrapperPane has no template, so just create an instance.
    const Str contents = R"(
  children: [
    TouchWrapperPane "TouchWrapperPane" {
      children: [
        PushButtonWidget "But" {},  # Wrapped Widget.
      ]
    }
  ]
)";
    auto twp = GetTouchWrapperPane(contents);
    EXPECT_TRUE(twp->GetPanes().empty());

    TouchInfo info;
    info.position.Set(0, 0, 0);
    info.radius = 1;
    info.root_node = twp;

    float dist = 100;
    auto w = twp->GetTouchedWidget(info, dist);
    EXPECT_NOT_NULL(w);
    EXPECT_EQ("But", w->GetName());
    EXPECT_EQ(0, dist);
}
