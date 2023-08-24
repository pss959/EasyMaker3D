#include "Enums/CSGOperation.h"
#include "Panels/CSGToolPanel.h"
#include "Panes/RadioButtonPane.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"

class CSGToolPanelTest : public PanelTestBase {};

TEST_F(CSGToolPanelTest, Defaults) {
    auto panel = ReadRealPanel<CSGToolPanel>("CSGToolPanel");
    EXPECT_EQ(CSGOperation::kUnion, panel->GetOperation());
    EXPECT_NULL(panel->GetFocusedPane());
}

TEST_F(CSGToolPanelTest, SetOperation) {
    auto panel = ReadRealPanel<CSGToolPanel>("CSGToolPanel");
    panel->SetOperation(CSGOperation::kDifference);
    EXPECT_EQ(CSGOperation::kDifference, panel->GetOperation());
}

TEST_F(CSGToolPanelTest, Show) {
    auto panel = ReadRealPanel<CSGToolPanel>("CSGToolPanel");
    EXPECT_FALSE(panel->IsShown());
    panel->SetIsShown(true);
    EXPECT_TRUE(panel->IsShown());
}

TEST_F(CSGToolPanelTest, Change) {
    auto panel = ReadRealPanel<CSGToolPanel>("CSGToolPanel");

    size_t change_count = 0;
    panel->GetInteraction().AddObserver(
        "key", [&](const Str &name, ToolPanel::InteractionType type){
            ++change_count;
            EXPECT_EQ("Operation",                            name);
            EXPECT_EQ(ToolPanel::InteractionType::kImmediate, type);
        });

    // Toggle one of the radio buttons.
    auto but = FindTypedPane<RadioButtonPane>(*panel, "Intersection");
    but->SetState(true);
    EXPECT_EQ(1U, change_count);
}
