#include "Panels/ImportToolPanel.h"
#include "Tests/Panels/ToolPanelTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ImportToolPanelTest : public ToolPanelTestBase {
  protected:
    ImportToolPanelPtr panel;
    ImportToolPanelTest() {
        panel = InitPanel<ImportToolPanel>("ImportToolPanel");
        ObserveChanges(*panel);
    }
};

TEST_F(ImportToolPanelTest, Defaults) {
    EXPECT_NULL(panel->GetFocusedPane());
    EXPECT_FALSE(panel->IsCloseable());
}

TEST_F(ImportToolPanelTest, Show) {
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());
}

TEST_F(ImportToolPanelTest, Apply) {
    panel->SetStatus(Panel::Status::kVisible);
    ClickButtonPane("Accept");
    EXPECT_EQ(1U,           GetChangeInfo().count);
    EXPECT_EQ("Accept",     GetChangeInfo().name);
    EXPECT_EQ("kImmediate", GetChangeInfo().type);
}

TEST_F(ImportToolPanelTest, Error) {
    panel->SetStatus(Panel::Status::kVisible);

    // Displaying an error should show a DialogPanel with the error message.
    panel->DisplayImportError("Some error");
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    auto dialog = GetCurrentPanel();
    EXPECT_EQ("DialogPanel", dialog->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, dialog->GetStatus());

    // Closing the dialog should restore the ImportToolPanel.
    ClickButtonPane("Button0");
    EXPECT_EQ("OK", GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, dialog->GetStatus());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());
}
