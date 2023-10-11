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
    EXPECT_FALSE(panel->IsCloseable());
    EXPECT_TRUE(panel->IsMovable());
    EXPECT_FALSE(panel->IsResizable());
    EXPECT_FALSE(panel->CanGripHover());
    EXPECT_NULL(panel->GetGripWidget(Point2f(0, 0)));
}

TEST_F(CSGToolPanelTest, SetOperation) {
    panel->SetOperation(CSGOperation::kDifference);
    EXPECT_EQ(CSGOperation::kDifference, panel->GetOperation());
}

TEST_F(CSGToolPanelTest, Show) {
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());
}

TEST_F(CSGToolPanelTest, Change) {
    pi.ActivateRadioButtonPane("Intersection");
    EXPECT_EQ(CSGOperation::kIntersection, panel->GetOperation());
    EXPECT_EQ(1U,           GetChangeInfo().count);
    EXPECT_EQ("Operation",  GetChangeInfo().name);
    EXPECT_EQ("kImmediate", GetChangeInfo().type);
}
