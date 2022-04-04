#include "Defaults.h"
#include "Panes/TextPane.h"
#include "SG/Search.h"
#include "SceneTestBase.h"
#include "Testing.h"

class PaneTest : public SceneTestBase {
};

TEST_F(PaneTest, TextPane) {
    EnableKLog("pq");  // XXXX

    const std::string input = ReadDataFile("Panes.mvn");
    SG::ScenePtr scene = ReadScene(input);
    TextPanePtr text = SG::FindTypedNodeInScene<TextPane>(*scene, "Text");
    text->SetText("ABC");
    text->SetFontSize(22);
    EXPECT_EQ(Vector2f(24.375f, 12.f), text->GetBaseSize());

    text->SetFontSize(44);
    EXPECT_EQ(Vector2f(48.75f, 24.f), text->GetBaseSize());
}

// XXXX Make sure to test all Pane functions that issue PaneChanged() to make
// XXXX sure sizes update.
