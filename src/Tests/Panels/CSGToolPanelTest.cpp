#include "Enums/CSGOperation.h"
#include "Panels/CSGToolPanel.h"
#include "Panes/RadioButtonPane.h"
#include "Tests/Panels/ToolPanelTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class CSGToolPanelTest : public ToolPanelTestBase {
  protected:
    CSGToolPanelPtr panel;
    CSGToolPanelTest() {
        panel = InitPanel<CSGToolPanel>("CSGToolPanel");
        ObserveChanges(*panel);
    }
};

TEST_F(CSGToolPanelTest, Defaults) {
    EXPECT_EQ(CSGOperation::kUnion, panel->GetOperation());
    EXPECT_NULL(panel->GetFocusedPane());
}

TEST_F(CSGToolPanelTest, SetOperation) {
    panel->SetOperation(CSGOperation::kDifference);
    EXPECT_EQ(CSGOperation::kDifference, panel->GetOperation());
}

TEST_F(CSGToolPanelTest, Show) {
    EXPECT_FALSE(panel->IsShown());
    panel->SetIsShown(true);
    EXPECT_TRUE(panel->IsShown());
}

TEST_F(CSGToolPanelTest, Change) {
    // Toggle one of the radio buttons.
    auto but = FindTypedPane<RadioButtonPane>("Intersection");
    but->SetState(true);
    EXPECT_EQ(1U,           GetChangeInfo().count);
    EXPECT_EQ("Operation",  GetChangeInfo().name);
    EXPECT_EQ("kImmediate", GetChangeInfo().type);
}
