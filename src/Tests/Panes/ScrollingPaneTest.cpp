#include "Base/Event.h"
#include "Panes/ScrollingPane.h"
#include "Panes/SpacerPane.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"
#include "Util/String.h"

/// \ingroup Tests
class ScrollingPaneTest : public PaneTestBase {
  protected:
    ScrollingPanePtr GetScrollingPane(const Str &clip_contents = "") {
        // ScrollingPane requires a ClipPane for the "contents" field.
        const Str contents =
            R"(min_size: 200 200,
               contents: CLONE "T_ClipPane" "CP" { )" + clip_contents + " }";
        return ReadRealPane<ScrollingPane>("ScrollingPane", contents);
    }
};

TEST_F(ScrollingPaneTest, Defaults) {
    auto scp = GetScrollingPane();
    EXPECT_EQ("CP",             scp->GetContentsPane()->GetName());
    EXPECT_EQ(16,               scp->GetScrollBarWidth());
    EXPECT_EQ(scp.get(),        scp->GetInteractor());
    EXPECT_EQ(scp->GetBorder(), scp->GetFocusBorder());
    EXPECT_NULL(scp->GetActivationWidget());
}

TEST_F(ScrollingPaneTest, IsValid) {
    SetParseTypeName("ScrollingPane");
    TestInvalid("", "Missing contents pane");
}

TEST_F(ScrollingPaneTest, Scroll) {
    // Make the ClipPane contents large enough to require scrolling. Add some
    // spacers to test ScrollToShowSubPane().
    auto scp = GetScrollingPane("min_size: 100 400");
    auto spp = ParseTypedObject<SpacerPane>("SpacerPane { min_size: 100 50 }");
    Pane::PaneVec spacers;
    for (size_t i = 0; i < 40; ++i)
        spacers.push_back(spp->CloneTyped<SpacerPane>(true));
    scp->GetContentsPane()->ReplacePanes(spacers);

    scp->SetLayoutSize(Vector2f(200, 200));
    EXPECT_EQ(Vector2f(200,  200), scp->GetBaseSize());
    EXPECT_EQ(Vector2f(200,  200), scp->GetLayoutSize());
    EXPECT_EQ(Vector2f(100,  400), scp->GetContentsPane()->GetBaseSize());
    EXPECT_EQ(Vector2f(100, 2000), scp->GetContentsPane()->GetUnclippedSize());
    EXPECT_EQ(0, scp->GetScrollPosition());

    scp->ScrollTo(.2f);
    EXPECT_CLOSE(.2f, scp->GetScrollPosition());

    // Should clamp.
    scp->ScrollTo(1.2f);
    EXPECT_EQ(1, scp->GetScrollPosition());
    scp->ScrollTo(-.1f);
    EXPECT_EQ(0, scp->GetScrollPosition());

    // ScrollBy() is relative to the scrolling speed and layout size.
    scp->ScrollBy(.4f);
    EXPECT_CLOSE(.01778f, scp->GetScrollPosition());
    scp->SetLayoutSize(Vector2f(200, 400));
    scp->ScrollTo(0);
    scp->ScrollBy(.8f);
    EXPECT_CLOSE(.08f, scp->GetScrollPosition());

    scp->ScrollToShowSubPane(*spacers[0]);
    EXPECT_CLOSE(0,    scp->GetScrollPosition());
    scp->ScrollToShowSubPane(*spacers[20]);
    EXPECT_CLOSE(.5f,  scp->GetScrollPosition());
    scp->ScrollToShowSubPane(*spacers[39]);
    EXPECT_CLOSE(1,    scp->GetScrollPosition());
    scp->ScrollToShowSubPane(*spacers[10]);
    EXPECT_CLOSE(.25f, scp->GetScrollPosition());

    // Handle scrolling events.
    Event event;
    event.device = Event::Device::kKeyboard;
    event.flags.Set(Event::Flag::kKeyPress);
    event.key_name = "Up";
    EXPECT_TRUE(scp->HandleEvent(event));
    EXPECT_CLOSE(.2f, scp->GetScrollPosition());

    event.key_name = "Down";
    EXPECT_TRUE(scp->HandleEvent(event));
    EXPECT_CLOSE(.25f, scp->GetScrollPosition());

    event.flags.SetAll(false);
    event.flags.Set(Event::Flag::kPosition1D);
    event.position1D = -.4f;
    EXPECT_TRUE(scp->HandleEvent(event));
    EXPECT_CLOSE(.246f, scp->GetScrollPosition());
}
