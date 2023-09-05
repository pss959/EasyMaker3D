#include "Panes/BoxPane.h"
#include "Panes/ButtonPane.h"
#include "Panes/SpacerPane.h"
#include "Place/TouchInfo.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"
#include "Util/General.h"
#include "Util/String.h"
#include "Util/Tuning.h"
#include "Widgets/Widget.h"
#include "Widgets/PushButtonWidget.h"

/// \ingroup Tests
class BoxPaneTest : public PaneTestBase {
  protected:
    /// BoxPane has no template, so this just creates an instance.
    BoxPanePtr GetBoxPane(const Str &contents = "");
};

BoxPanePtr BoxPaneTest::GetBoxPane(const Str &contents) {
    const Str s = R"(
  children: [
    <"nodes/templates/Panes/AllPanes.emd">,
    BoxPane "BoxPane" { @CONTENTS@ }
  ]
)";
    return ReadRealNode<BoxPane>(
        Util::ReplaceString(s, "@CONTENTS@", contents), "BoxPane");
};

TEST_F(BoxPaneTest, Defaults) {
    auto box = GetBoxPane();
    EXPECT_EQ(BoxPane::Orientation::kVertical, box->GetOrientation());
    EXPECT_EQ(0,                               box->GetPadding());
    EXPECT_EQ(0,                               box->GetSpacing());
    EXPECT_NULL(box->GetInteractor());
}

TEST_F(BoxPaneTest, Fields) {
    auto box = GetBoxPane(
        R"(orientation: "kHorizontal", padding: 2, spacing: 1.5)");
    EXPECT_EQ(BoxPane::Orientation::kHorizontal, box->GetOrientation());
    EXPECT_EQ(2,                                 box->GetPadding());
    EXPECT_EQ(1.5f,                              box->GetSpacing());
}

TEST_F(BoxPaneTest, LayoutVBox) {
    const Str contents = R"(
  padding: 4,
  spacing: 10,
  panes: [
    SpacerPane "Spacer0" { min_size: 8 15 },
    SpacerPane "Spacer1" { resize_flags: "kWidth|kHeight" },
    SpacerPane "Spacer2" {},
    SpacerPane "Spacer3" { resize_flags: "kHeight",
                           disabled_flags: "kTraversal" },
  ]
)";
    auto box = GetBoxPane(contents);
    auto sp0 = SG::FindTypedNodeUnderNode<SpacerPane>(*box, "Spacer0");
    auto sp1 = SG::FindTypedNodeUnderNode<SpacerPane>(*box, "Spacer1");
    auto sp2 = SG::FindTypedNodeUnderNode<SpacerPane>(*box, "Spacer2");
    auto sp3 = SG::FindTypedNodeUnderNode<SpacerPane>(*box, "Spacer3");

    const Vector2f base_size((2 * 4) + 8,
                             (2 * 4) + (2 * 10) + (1 + 15 + 1));
    EXPECT_EQ(base_size, box->GetBaseSize());
    EXPECT_EQ(Vector2f::Zero(), box->GetLayoutSize());  // Not layed out yet.

    box->SetLayoutSize(base_size);
    EXPECT_EQ(base_size, box->GetBaseSize());
    EXPECT_EQ(base_size, box->GetLayoutSize());
    EXPECT_EQ(Vector2f(8, 15), sp0->GetLayoutSize());
    EXPECT_EQ(Vector2f(8,  1), sp1->GetLayoutSize());
    EXPECT_EQ(Vector2f(1,  1), sp2->GetLayoutSize());
    EXPECT_EQ(Vector2f(0,  0), sp3->GetLayoutSize());

    // Increase the layout size of the box. Only Spacer1 should change
    // size. Note that Spacer3 should resize, but it is not enabled.
    const Vector2f new_size = base_size + Vector2f(100, 200);
    box->SetLayoutSize(new_size);
    EXPECT_EQ(base_size, box->GetBaseSize());
    EXPECT_EQ(new_size,  box->GetLayoutSize());
    EXPECT_EQ(Vector2f(8,    15), sp0->GetLayoutSize());
    EXPECT_EQ(Vector2f(108, 201), sp1->GetLayoutSize());
    EXPECT_EQ(Vector2f(1,     1), sp2->GetLayoutSize());
    EXPECT_EQ(Vector2f(0,     0), sp3->GetLayoutSize());

    const auto panes = box->GetPotentialInteractiveSubPanes();
    EXPECT_EQ(4U, panes.size());
    EXPECT_TRUE(Util::Contains(panes, sp0));
    EXPECT_TRUE(Util::Contains(panes, sp1));
    EXPECT_TRUE(Util::Contains(panes, sp2));
    EXPECT_TRUE(Util::Contains(panes, sp3));
}

TEST_F(BoxPaneTest, LayoutHBox) {
    const Str contents = R"(
  orientation: "kHorizontal",
  padding: 4,
  spacing: 10,
  panes: [
    SpacerPane "Spacer0" { min_size: 15 8 },
    SpacerPane "Spacer1" { resize_flags: "kWidth|kHeight" },
    SpacerPane "Spacer2" {},
    SpacerPane "Spacer3" { resize_flags: "kWidth",
                           disabled_flags: "kTraversal" },
  ]
)";
    auto box = GetBoxPane(contents);
    auto sp0 = SG::FindTypedNodeUnderNode<SpacerPane>(*box, "Spacer0");
    auto sp1 = SG::FindTypedNodeUnderNode<SpacerPane>(*box, "Spacer1");
    auto sp2 = SG::FindTypedNodeUnderNode<SpacerPane>(*box, "Spacer2");
    auto sp3 = SG::FindTypedNodeUnderNode<SpacerPane>(*box, "Spacer3");

    const Vector2f base_size((2 * 4) + (2 * 10) + (1 + 15 + 1),
                             (2 * 4) + 8);
    EXPECT_EQ(base_size, box->GetBaseSize());
    EXPECT_EQ(Vector2f::Zero(), box->GetLayoutSize());  // Not layed out yet.

    box->SetLayoutSize(base_size);
    EXPECT_EQ(base_size, box->GetBaseSize());
    EXPECT_EQ(base_size, box->GetLayoutSize());
    EXPECT_EQ(Vector2f(15, 8), sp0->GetLayoutSize());
    EXPECT_EQ(Vector2f(1,  8), sp1->GetLayoutSize());
    EXPECT_EQ(Vector2f(1,  1), sp2->GetLayoutSize());
    EXPECT_EQ(Vector2f(0,  0), sp3->GetLayoutSize());

    // Increase the layout size of the box. Only Spacer1 should change
    // size. Note that Spacer3 should resize, but it is not enabled.
    const Vector2f new_size = base_size + Vector2f(200, 100);
    box->SetLayoutSize(new_size);
    EXPECT_EQ(base_size, box->GetBaseSize());
    EXPECT_EQ(new_size,  box->GetLayoutSize());
    EXPECT_EQ(Vector2f(15,    8), sp0->GetLayoutSize());
    EXPECT_EQ(Vector2f(201, 108), sp1->GetLayoutSize());
    EXPECT_EQ(Vector2f(1,     1), sp2->GetLayoutSize());
    EXPECT_EQ(Vector2f(0,     0), sp3->GetLayoutSize());
}

TEST_F(BoxPaneTest, Touch) {
    const Str contents = R"(
  orientation: "kHorizontal",
  panes: [
    CLONE "T_ButtonPane" "Button0" { min_size: 20 20 },
    CLONE "T_ButtonPane" "Button1" { min_size:  0 20 },  # Disabled below.
    CLONE "T_ButtonPane" "Button2" { min_size: 80 20 },
  ]
)";
    auto box = GetBoxPane(contents);
    auto but0 = SG::FindTypedNodeUnderNode<ButtonPane>(*box, "Button0");
    auto but1 = SG::FindTypedNodeUnderNode<ButtonPane>(*box, "Button1");
    auto but2 = SG::FindTypedNodeUnderNode<ButtonPane>(*box, "Button2");
    but1->SetInteractionEnabled(false);
    box->SetLayoutSize(Vector2f(60, 20));

    TouchInfo info;
    info.radius = .01f;
    info.root_node = box;

    const float z = TK::kPaneZOffset;
    float dist;

    // Touch the middle of Button0.
    dist = 1000;
    info.position.Set(-.3333f, 0, z);
    auto tw = box->GetTouchedWidget(info, dist);
    ASSERT_NOT_NULL(tw);
    EXPECT_EQ(&but0->GetButton(), tw.get());

    // Touch the middle of Button2.
    dist = 1000;
    info.position.Set(.5f, 0, z);
    tw = box->GetTouchedWidget(info, dist);
    ASSERT_NOT_NULL(tw);
    EXPECT_EQ(&but2->GetButton(), tw.get());

    // Miss completely.
    dist = 1000;
    info.position.Set(1.5f, 0, z);
    tw = box->GetTouchedWidget(info, dist);
    EXPECT_NULL(tw);
}
