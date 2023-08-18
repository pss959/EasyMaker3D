#include "Panes/DropdownPane.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/UnitTestTypeChanger.h"
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
    EXPECT_TRUE(dd->GetPotentialInteractiveSubPanes().empty());
}

TEST_F(DropdownPaneTest, SetChoices) {
    auto dd = GetDropdownPane();

    dd->SetChoices(StrVec{"Abcd", "Efgh Ijklmn", "Op Qrstu"}, 2);
    EXPECT_EQ(2,          dd->GetChoiceIndex());
    EXPECT_EQ("Op Qrstu", dd->GetChoice());

    dd->SetChoiceFromString("Efgh Ijklmn");
    EXPECT_EQ(1,             dd->GetChoiceIndex());
    EXPECT_EQ("Efgh Ijklmn", dd->GetChoice());

    TEST_THROW(dd->SetChoiceFromString("Bad Choice"), AssertException,
               "No such choice");
}

#if XXXX
TEST_F(DropdownPaneTest, Dropdown) {
    // Override this setting for this test; need to build font images.
    UnitTestTypeChanger uttc(Util::AppType::kInteractive);

    SG::ScenePtr scene = ReadPaneScene();
    auto dd = SG::FindTypedNodeInScene<DropdownPane>(*scene, "Dropdown");

    StrVec choices{ "Abcd", "Efgh Ijklmn", "Op Qrstu" };
    dd->SetChoices(choices, 2);
    dd->SetLayoutSize(Vector2f(100, 20));

    EXPECT_EQ(2, dd->GetChoiceIndex());
    EXPECT_EQ("Op Qrstu", dd->GetChoice());

    // The base size of the DropdownPane is the size of the largest choice.
    EXPECT_EQ(Vector2f(88.425f, 20), dd->GetBaseSize());

    // Changing the choice should not affect the base size.
    dd->SetChoice(0);
    EXPECT_EQ(0, dd->GetChoiceIndex());
    EXPECT_EQ("Abcd", dd->GetChoice());
    EXPECT_EQ(Vector2f(88.425f, 20), dd->GetBaseSize());

    // Each choice button in the dropdown should have a non-zero layout size.
    for (const auto &but: dd->GetMenuPane().GetContentsPane()->GetPanes()) {
        const Vector2f &ls = but->GetLayoutSize();
        EXPECT_NE(0, ls[0]);
        EXPECT_NE(0, ls[1]);
    }

    dd->SetChoice(1);
    EXPECT_EQ(1, dd->GetChoiceIndex());
    EXPECT_EQ("Efgh Ijklmn", dd->GetChoice());

    dd->SetChoiceFromString("Op Qrstu");
    EXPECT_EQ(2, dd->GetChoiceIndex());
    EXPECT_EQ("Op Qrstu", dd->GetChoice());
}
#endif
