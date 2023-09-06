#include "Base/Event.h"
#include "Panes/DropdownPane.h"
#include "Panes/ScrollingPane.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"
#include "Util/Assert.h"

/// \ingroup Tests
class DropdownPaneTest : public PaneTestBase {
  protected:
    DropdownPanePtr GetDropdownPane(const Str &contents = "") {
        return ReadRealPane<DropdownPane>("DropdownPane", contents);
    }
};

TEST_F(DropdownPaneTest, Defaults) {
    auto dd = GetDropdownPane();
    EXPECT_EQ(".",              dd->GetChoice());
    EXPECT_EQ(-1,               dd->GetChoiceIndex());
    EXPECT_EQ(dd.get(),         dd->GetInteractor());
    EXPECT_NOT_NULL(dd->GetActivationWidget());
    EXPECT_EQ(dd->GetBorder(),  dd->GetFocusBorder());
    Pane::PaneVec panes;
    dd->GetFocusableSubPanes(panes);
    EXPECT_TRUE(panes.empty());
}

TEST_F(DropdownPaneTest, SetChoices) {
    auto dd = GetDropdownPane();

    // Test notification for some of these.
    Str    cur_choice;
    size_t change_count = 0;
    dd->GetChoiceChanged().AddObserver("key", [&](const Str &c){
        cur_choice = c;
        ++change_count;
    });
    EXPECT_EQ(0U,  change_count);
    EXPECT_EQ("",  cur_choice);

    // Should not notify.
    dd->SetChoices(StrVec{"Abcd", "Efgh Ijklmn", "Op Qrstu"}, 2);
    EXPECT_EQ(2,          dd->GetChoiceIndex());
    EXPECT_EQ("Op Qrstu", dd->GetChoice());
    EXPECT_EQ(0U,         change_count);
    EXPECT_EQ("",         cur_choice);

    // Should notify.
    dd->SetChoiceFromString("Efgh Ijklmn", true);  // Notify.
    EXPECT_EQ(1,             dd->GetChoiceIndex());
    EXPECT_EQ("Efgh Ijklmn", dd->GetChoice());
    EXPECT_EQ(1U,            change_count);
    EXPECT_EQ("Efgh Ijklmn", cur_choice);

    // Should not notify.
    dd->SetChoices(StrVec(), 10);  // Index should be ignored.
    EXPECT_EQ(-1,            dd->GetChoiceIndex());
    EXPECT_EQ(".",           dd->GetChoice());
    EXPECT_EQ(1U,            change_count);
    EXPECT_EQ("Efgh Ijklmn", cur_choice);

    TEST_THROW(dd->SetChoiceFromString("Bad Choice"), AssertException,
               "No such choice");
}

TEST_F(DropdownPaneTest, IsValid) {
    SetParseTypeName("DropdownPane");
    TestInvalid(R"(choices: ["A", "B"])", "No initial choice");
    TestInvalid(R"(choices: ["A", "B"], choice_index: 2)",
                "Choice index out of range");
}

TEST_F(DropdownPaneTest, Activate) {
    auto dd = GetDropdownPane(R"(choices: ["A", "B", "C"], choice_index: 2)");
    EXPECT_EQ(2,   dd->GetChoiceIndex());
    EXPECT_EQ("C", dd->GetChoice());

    EXPECT_FALSE(dd->IsActive());

    dd->Activate();
    EXPECT_TRUE(dd->IsActive());

    dd->Deactivate();
    EXPECT_FALSE(dd->IsActive());
}

TEST_F(DropdownPaneTest, LayoutSize) {
    auto dd = GetDropdownPane(R"(choices: ["A", "B", "C"], choice_index: 1)");

    dd->SetLayoutSize(Vector2f(100, 20));

    // The base size of the DropdownPane is the size of the largest choice.
    EXPECT_VECS_CLOSE2(Vector2f(40.8, 20), dd->GetBaseSize());

    // Changing the choice should not affect the base size.
    dd->SetChoice(0);
    EXPECT_VECS_CLOSE2(Vector2f(40.8, 20), dd->GetBaseSize());

    // Each choice button in the dropdown should have a positive layout size.
    for (const auto &but: dd->GetMenuPane().GetContentsPane()->GetPanes()) {
        const Vector2f &ls = but->GetLayoutSize();
        EXPECT_LT(0, ls[0]);
        EXPECT_LT(0, ls[1]);
    }
}

TEST_F(DropdownPaneTest, HandleEvent) {
    auto dd = GetDropdownPane(R"(choices: ["A", "B", "C"], choice_index: 1)");
    EXPECT_EQ(1,   dd->GetChoiceIndex());
    EXPECT_EQ("B", dd->GetChoice());

    Str    cur_choice;
    size_t change_count = 0;
    dd->GetChoiceChanged().AddObserver("key", [&](const Str &c){
        cur_choice = c;
        ++change_count;
    });

    Event event;
    event.device = Event::Device::kKeyboard;
    event.flags.Set(Event::Flag::kKeyPress);
    event.key_name = "Up";

    // Nothing happens when not active.
    EXPECT_FALSE(dd->HandleEvent(event));
    EXPECT_EQ(1,   dd->GetChoiceIndex());
    EXPECT_EQ("B", dd->GetChoice());

    dd->Activate();
    EXPECT_TRUE(dd->HandleEvent(event));
    EXPECT_EQ(0,   dd->GetChoiceIndex());
    EXPECT_EQ("A", dd->GetChoice());

    // Cannot go up past 0.
    EXPECT_TRUE(dd->HandleEvent(event));
    EXPECT_EQ(0,   dd->GetChoiceIndex());
    EXPECT_EQ("A", dd->GetChoice());

    event.key_name = "Down";
    EXPECT_TRUE(dd->HandleEvent(event));
    EXPECT_EQ(1,   dd->GetChoiceIndex());
    EXPECT_EQ("B", dd->GetChoice());

    EXPECT_TRUE(dd->HandleEvent(event));
    EXPECT_EQ(2,   dd->GetChoiceIndex());
    EXPECT_EQ("C", dd->GetChoice());

    // Cannot go down past last choice.
    EXPECT_TRUE(dd->HandleEvent(event));
    EXPECT_EQ(2,   dd->GetChoiceIndex());
    EXPECT_EQ("C", dd->GetChoice());

    // Space or Enter changes the current choice and deactivates.
    EXPECT_EQ(0U, change_count);
    EXPECT_EQ("", cur_choice);
    event.key_name = " ";
    EXPECT_TRUE(dd->HandleEvent(event));
    EXPECT_EQ(1U,  change_count);
    EXPECT_EQ("C", cur_choice);
    EXPECT_FALSE(dd->IsActive());

    dd->Activate();
    event.key_name = "Up";
    EXPECT_TRUE(dd->HandleEvent(event));
    event.key_name = "Enter";
    EXPECT_TRUE(dd->HandleEvent(event));
    EXPECT_EQ(2U,  change_count);
    EXPECT_EQ("B", cur_choice);
    EXPECT_FALSE(dd->IsActive());

    // Other keys are not handled.
    dd->Activate();
    event.key_name = "a";
    EXPECT_FALSE(dd->HandleEvent(event));

    // Escape closes the DropdownPane to deactivate.
    event.key_name = "Escape";
    EXPECT_TRUE(dd->HandleEvent(event));
    EXPECT_FALSE(dd->IsActive());
}
