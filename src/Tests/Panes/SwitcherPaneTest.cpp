#include "Panes/SwitcherPane.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"
#include "Util/Tuning.h"

/// \ingroup Tests
class SwitcherPaneTest : public PaneTestBase {};

TEST_F(SwitcherPaneTest, Switching) {
    // SwitcherPane has no template, so just create an instance.
    const Str contents = R"(
  children: [
    <"nodes/templates/Panes/AllPanes.emd">,
    SwitcherPane "SwitcherPane" {
      panes: [
        CLONE "T_ButtonPane" "But0" { min_size: 10 10 },
        CLONE "T_ButtonPane" "But1" { min_size: 10 10 },
        CLONE "T_ButtonPane" "But2" { min_size: 10 10 },
      ]
    }
  ]
)";
    auto sw = ReadRealNode<SwitcherPane>(contents, "SwitcherPane");
    EXPECT_EQ(3U, sw->GetPanes().size());
    EXPECT_EQ(-1, sw->GetIndex());

    size_t change_count = 0;
    sw->GetBaseSizeChanged().AddObserver("key", [&]{ ++change_count; });

    const Vector2f v10x10(10, 10);
    sw->SetLayoutSize(v10x10);
    EXPECT_EQ(v10x10, sw->GetBaseSize());
    EXPECT_EQ(v10x10, sw->GetLayoutSize());

    EXPECT_EQ(-1, sw->GetIndex());
    EXPECT_EQ(0U, change_count);
    EXPECT_EQ(v10x10, sw->GetBaseSize());
    EXPECT_FALSE(sw->GetPanes()[0]->IsEnabled());
    EXPECT_FALSE(sw->GetPanes()[1]->IsEnabled());
    EXPECT_FALSE(sw->GetPanes()[2]->IsEnabled());

    sw->SetIndex(1);
    EXPECT_EQ(1U, change_count);
    EXPECT_EQ(v10x10, sw->GetBaseSize());
    EXPECT_FALSE(sw->GetPanes()[0]->IsEnabled());
    EXPECT_TRUE(sw->GetPanes()[1]->IsEnabled());
    EXPECT_FALSE(sw->GetPanes()[2]->IsEnabled());

    sw->SetLayoutSize(v10x10);
    EXPECT_EQ(v10x10, sw->GetPanes()[1]->GetBaseSize());
    EXPECT_EQ(v10x10, sw->GetPanes()[1]->GetLayoutSize());
    EXPECT_EQ(Vector3f(0, 0, TK::kPaneZOffset),
              sw->GetPanes()[1]->GetTranslation());
    sw->SetIndex(1);
}
