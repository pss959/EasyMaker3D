#include "Panes/IconPane.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class IconPaneTest : public PaneTestBase {
  protected:
    IconPanePtr GetIconPane(const Str &contents = "") {
        return ReadRealPane<IconPane>("IconPane", contents);
    }
};

TEST_F(IconPaneTest, Defaults) {
    auto icon = GetIconPane(R"(icon_name: "UpIcon")");
    EXPECT_EQ("UpIcon", icon->GetIconName());
}

TEST_F(IconPaneTest, Set) {
    auto icon = GetIconPane(R"(icon_name: "UpIcon")");
    icon->SetIconName("DownIcon");
    EXPECT_EQ("DownIcon", icon->GetIconName());
}

TEST_F(IconPaneTest, IsValid) {
    SetParseTypeName("IconPane");
    TestInvalid("", "No icon name specified");
}
