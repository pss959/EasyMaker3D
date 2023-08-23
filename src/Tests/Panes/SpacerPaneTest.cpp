#include "Panes/SpacerPane.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class SpacerPaneTest : public PaneTestBase {};

TEST_F(SpacerPaneTest, SetSpace) {
    // SpacerPane has no template, so just create an instance.
    const Str contents = R"(children: [ SpacerPane "SpacerPane" {} ])";

    auto sp = ReadRealNode<SpacerPane>(contents, "SpacerPane");
    EXPECT_EQ(Vector2f(1, 1), sp->GetMinSize());
    EXPECT_EQ(Vector2f(1, 1), sp->GetBaseSize());
    EXPECT_EQ(Vector2f(1, 1), sp->GetCurrentBaseSize());

    sp->SetSpace(Vector2f(10, 20));
    EXPECT_EQ(Vector2f(10, 20), sp->GetMinSize());
    EXPECT_EQ(Vector2f(10, 20), sp->GetBaseSize());
    EXPECT_EQ(Vector2f(10, 20), sp->GetCurrentBaseSize());
    EXPECT_TRUE(sp->IsBaseSizeUpToDate());

    sp->SetLayoutSize(Vector2f(40, 30));
    EXPECT_EQ(Vector2f(40, 30), sp->GetLayoutSize());
    sp->ResetLayoutSize();
    EXPECT_EQ(Vector2f(0, 0), sp->GetLayoutSize());
}
