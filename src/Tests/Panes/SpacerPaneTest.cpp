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

    sp->SetSpace(Vector2f(10, 20));
    EXPECT_EQ(Vector2f(10, 20), sp->GetMinSize());
}
