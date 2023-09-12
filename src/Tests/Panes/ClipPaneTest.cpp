#include "Panes/ClipPane.h"
#include "Panes/ButtonPane.h"
#include "Panes/SpacerPane.h"
#include "Place/TouchInfo.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"
#include "Util/String.h"
#include "Util/Tuning.h"
#include "Widgets/Widget.h"
#include "Widgets/PushButtonWidget.h"

/// \ingroup Tests
class ClipPaneTest : public PaneTestBase {
  protected:
    ClipPanePtr GetClipPane(const Str &contents = "") {
        return ReadRealPane<ClipPane>("ClipPane", contents);
    }
};

TEST_F(ClipPaneTest, Defaults) {
    auto cp = GetClipPane();
    EXPECT_EQ(Vector2f(1, 1), cp->GetBaseSize());
    EXPECT_EQ(Vector2f(1, 1), cp->GetUnclippedSize());
    EXPECT_EQ(Vector2f(0, 0), cp->GetContentsOffset());
}

TEST_F(ClipPaneTest, Touch) {
    // Create a ClipPane with some clipped contents.
    const Str contents = R"(
  panes: [
    CLONE "T_ButtonPane" "Button0" { min_size: 80 40 },
    CLONE "T_ButtonPane" "Button1" { min_size: 80 40 },
    CLONE "T_ButtonPane" "Button2" { min_size: 80 40 },
    CLONE "T_ButtonPane" "Button3" { min_size: 80 40 },
    CLONE "T_ButtonPane" "Button4" { min_size: 80 40 },
  ]
)";
    auto cp = GetClipPane(contents);
    auto but0 = SG::FindTypedNodeUnderNode<ButtonPane>(*cp, "Button0");
    auto but1 = SG::FindTypedNodeUnderNode<ButtonPane>(*cp, "Button1");
    auto but2 = SG::FindTypedNodeUnderNode<ButtonPane>(*cp, "Button2");
    auto but3 = SG::FindTypedNodeUnderNode<ButtonPane>(*cp, "Button3");
    auto but4 = SG::FindTypedNodeUnderNode<ButtonPane>(*cp, "Button4");
    but1->SetEnabled(false);

    // Make the layout wide enough for all buttons but high enough for only one
    // and part of another.
    cp->SetLayoutSize(Vector2f(100, 70));

    TouchInfo info;
    info.radius = .01f;
    info.root_node = cp;

    const float z = TK::kPaneZOffset;
    float dist;

    // Touch the middle of the top button.
    dist = 1000;
    info.position.Set(-.5f, .5f, z);
    auto tw = cp->GetTouchedWidget(info, dist);
    ASSERT_NOT_NULL(tw);
    EXPECT_EQ(&but0->GetButton(), tw.get());

    // Touch the middle of the second button. It should miss Button1 because it
    // is not enabled and hit Button2.
    dist = 1000;
    info.position.Set(-.5f, -.25f, z);
    tw = cp->GetTouchedWidget(info, dist);
    ASSERT_NOT_NULL(tw);
    EXPECT_EQ(&but2->GetButton(), tw.get());

    // Touch the middle of the third button. It should fail due to clipping.
    dist = 1000;
    info.position.Set(-.1f, -.75f, z);
    tw = cp->GetTouchedWidget(info, dist);
    EXPECT_NULL(tw);
}
