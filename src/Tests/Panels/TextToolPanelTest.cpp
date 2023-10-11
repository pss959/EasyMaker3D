#include "Panels/TextToolPanel.h"
#include "Panes/TextInputPane.h"
#include "Panes/TextPane.h"
#include "Tests/Panels/ToolPanelTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class TextToolPanelTest : public ToolPanelTestBase {
  protected:
    TextToolPanelPtr panel;
    TextToolPanelTest() {
        panel = InitPanel<TextToolPanel>("TextToolPanel");
        ObserveChanges(*panel);
    }
};

TEST_F(TextToolPanelTest, Defaults) {
    EXPECT_EQ(".",          panel->GetTextString());
    EXPECT_EQ("Arial-Bold", panel->GetFontName());
    EXPECT_EQ(0,            panel->GetCharSpacing());
    EXPECT_NULL(panel->GetFocusedPane());
    EXPECT_FALSE(panel->IsCloseable());
    EXPECT_TRUE(panel->IsMovable());
    EXPECT_TRUE(panel->IsResizable());
    EXPECT_FALSE(panel->CanGripHover());
    EXPECT_NULL(panel->GetGripWidget(Point2f(0, 0)));
}

TEST_F(TextToolPanelTest, SetValues) {
    panel->SetValues("Hello!", "Verdana-Regular", 1.5f);
    EXPECT_EQ("Hello!",          panel->GetTextString());
    EXPECT_EQ("Verdana-Regular", panel->GetFontName());
    EXPECT_EQ(1.5f,              panel->GetCharSpacing());
}

TEST_F(TextToolPanelTest, Show) {
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());
}

TEST_F(TextToolPanelTest, Change) {
    panel->SetStatus(Panel::Status::kVisible);

    auto input = pi.SetTextInput("Text", "Some Text");
    auto disp  = FindTypedPane<TextPane>("Display");
    auto msg   = FindTypedPane<TextPane>("Message");

    // Change to valid text.
    EXPECT_TRUE(input->IsTextValid());
    EXPECT_EQ("Some Text", input->GetText());
    EXPECT_EQ("Some Text", disp->GetText());
    EXPECT_TRUE(msg->GetText().empty());

    // Apply.
    pi.ClickButtonPane("Apply");
    EXPECT_EQ(1U,           GetChangeInfo().count);
    EXPECT_EQ("Apply",      GetChangeInfo().name);
    EXPECT_EQ("kImmediate", GetChangeInfo().type);

    // Set values to create a baseline for new changes.
    panel->SetValues(panel->GetTextString(), panel->GetFontName(),
                     panel->GetCharSpacing());

    // Change to font.
    pi.ChangeDropdownChoice("Font", "Verdana-Regular");
    EXPECT_EQ("Verdana-Regular", panel->GetFontName());

    // Apply.
    pi.ClickButtonPane("Apply");
    EXPECT_EQ(2U,           GetChangeInfo().count);
    EXPECT_EQ("Apply",      GetChangeInfo().name);
    EXPECT_EQ("kImmediate", GetChangeInfo().type);

    // Set values to create a baseline for new changes.
    panel->SetValues(panel->GetTextString(), panel->GetFontName(),
                     panel->GetCharSpacing());

    // Change character spacing.
    pi.DragSlider("Spacing", Vector2f(.4f, 0));
    EXPECT_CLOSE(2.1f, panel->GetCharSpacing());

    // Apply.
    pi.ClickButtonPane("Apply");
    EXPECT_EQ(3U,           GetChangeInfo().count);
    EXPECT_EQ("Apply",      GetChangeInfo().name);
    EXPECT_EQ("kImmediate", GetChangeInfo().type);

    // Set values to create a baseline for new changes.
    panel->SetValues(panel->GetTextString(), panel->GetFontName(),
                     panel->GetCharSpacing());

    // Change to invalid text.
    const Str bad_text = "¶¡§";  // Invalid characters for FakeFontSystem.
    input = pi.SetTextInput("Text", bad_text);
    EXPECT_FALSE(input->IsTextValid());
    EXPECT_EQ(bad_text, input->GetText());
    EXPECT_EQ(bad_text, disp->GetText());
    EXPECT_TRUE(msg->GetText().contains("Invalid character"));
    EXPECT_FALSE(pi.IsButtonPaneEnabled("Apply"));
}
