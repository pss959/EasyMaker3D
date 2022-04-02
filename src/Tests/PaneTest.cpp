#include "Defaults.h"
#include "Panes/TextPane.h"
#include "SG/Search.h"
#include "SceneTestBase.h"
#include "Testing.h"

class PaneTest : public SceneTestBase {
};

TEST_F(PaneTest, TextPane) {
    const std::string input = ReadDataFile("Panes.mvn");
    SG::ScenePtr scene = ReadScene(input);
    TextPanePtr text = SG::FindTypedNodeInScene<TextPane>(*scene, "Text");
    text->SetText("ABC");
    text->SetFontSize(22);
    EXPECT_EQ(Vector2f(24.375f, 12.f), text->GetBaseSize());
}
