#include "Panels/TextToolPanel.h"
#include "Panes/TextInputPane.h"
#include "Panes/TextPane.h"
#include "Tests/Panels/ToolPanelTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class TextToolPanelTest : public ToolPanelTestBase {
  protected:
    TextToolPanelPtr panel;
    explicit TextToolPanelTest(bool need_text = false) :
        ToolPanelTestBase(need_text) {
        panel = InitPanel<TextToolPanel>("TextToolPanel");
        ObserveChanges(*panel);
    }
};

/// \ingroup Tests
class TextToolPanelTestWithText : public TextToolPanelTest {
  protected:
    TextToolPanelTestWithText() : TextToolPanelTest(true) {}
};

TEST_F(TextToolPanelTest, Defaults) {
    EXPECT_EQ(".",          panel->GetTextString());
    EXPECT_EQ("Arial-Bold", panel->GetFontName());
    EXPECT_EQ(0,            panel->GetCharSpacing());
    EXPECT_NULL(panel->GetFocusedPane());
    EXPECT_FALSE(panel->IsCloseable());
}

TEST_F(TextToolPanelTest, SetValues) {
    panel->SetValues("Hello!", "Verdana-Regular", 1.5f);
    EXPECT_EQ("Hello!",          panel->GetTextString());
    EXPECT_EQ("Verdana-Regular", panel->GetFontName());
    EXPECT_EQ(1.5f,              panel->GetCharSpacing());
}

TEST_F(TextToolPanelTest, Show) {
    EXPECT_FALSE(panel->IsShown());
    panel->SetIsShown(true);
    EXPECT_TRUE(panel->IsShown());
}

TEST_F(TextToolPanelTestWithText, Change) {
    panel->SetSize(Vector2f(300, 300));  // Required for text sizing.
    panel->SetIsShown(true);

    auto input = SetTextInput("Text", "Some Text");
    auto disp  = FindTypedPane<TextPane>("Display");
    auto msg   = FindTypedPane<TextPane>("Message");

    // Change to valid text.
    EXPECT_TRUE(input->IsTextValid());
    EXPECT_EQ("Some Text", input->GetText());
    EXPECT_EQ("Some Text", disp->GetText());
    EXPECT_TRUE(msg->GetText().empty());

    // Apply.
    ClickButtonPane("Apply");
    EXPECT_EQ(1U,           GetChangeInfo().count);
    EXPECT_EQ("Apply",      GetChangeInfo().name);
    EXPECT_EQ("kImmediate", GetChangeInfo().type);

    // Set values to create a baseline for new changes.
    panel->SetValues(panel->GetTextString(), panel->GetFontName(),
                     panel->GetCharSpacing());

    // Change to font.
    ChangeDropdownChoice("Font", "Verdana-Regular");
    EXPECT_EQ("Verdana-Regular", panel->GetFontName());

    // Apply.
    ClickButtonPane("Apply");
    EXPECT_EQ(2U,           GetChangeInfo().count);
    EXPECT_EQ("Apply",      GetChangeInfo().name);
    EXPECT_EQ("kImmediate", GetChangeInfo().type);

    // Set values to create a baseline for new changes.
    panel->SetValues(panel->GetTextString(), panel->GetFontName(),
                     panel->GetCharSpacing());

    // Change character spacing.
    DragSlider("Spacing", Vector2f(.4f, 0));
    EXPECT_CLOSE(2.1f, panel->GetCharSpacing());

    // Apply.
    ClickButtonPane("Apply");
    EXPECT_EQ(3U,           GetChangeInfo().count);
    EXPECT_EQ("Apply",      GetChangeInfo().name);
    EXPECT_EQ("kImmediate", GetChangeInfo().type);

    // Set values to create a baseline for new changes.
    panel->SetValues(panel->GetTextString(), panel->GetFontName(),
                     panel->GetCharSpacing());

    // Change to invalid text.
    const Str bad_text = "¶¡§";  // Characters not in font.
    input = SetTextInput("Text", bad_text);
    EXPECT_FALSE(input->IsTextValid());
    EXPECT_EQ(bad_text, input->GetText());
    EXPECT_EQ(bad_text, disp->GetText());
    EXPECT_TRUE(msg->GetText().contains("invalid character(s) for the font"));
    EXPECT_FALSE(IsButtonPaneEnabled("Apply"));
}
