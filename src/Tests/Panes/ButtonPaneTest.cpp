#include "Panes/ButtonPane.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"
#include "Widgets/PushButtonWidget.h"

/// \ingroup Tests
class ButtonPaneTest : public PaneTestBase {
  protected:
    ButtonPanePtr GetButtonPane(const std::string &contents = "") {
        return ReadRealPane<ButtonPane>("ButtonPane", contents);
    }
};

TEST_F(ButtonPaneTest, Defaults) {
    auto but = GetButtonPane();
    EXPECT_TRUE(but->IsNameRequired());
    EXPECT_EQ(but.get(),         but->GetInteractor());
    EXPECT_EQ(&but->GetButton(), but->GetActivationWidget().get());
    EXPECT_EQ(but->GetBorder(),  but->GetFocusBorder());
}

TEST_F(ButtonPaneTest, InteractionEnabled) {
    auto but = GetButtonPane();
    EXPECT_TRUE(but->GetButton().IsInteractionEnabled());

    but->SetInteractionEnabled(false);
    EXPECT_FALSE(but->GetButton().IsInteractionEnabled());
    EXPECT_NULL(but->GetFocusBorder());

    but->SetInteractionEnabled(true);
    EXPECT_TRUE(but->GetButton().IsInteractionEnabled());
    EXPECT_EQ(but->GetBorder(), but->GetFocusBorder());
}
