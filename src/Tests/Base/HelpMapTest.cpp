#include "Base/HelpMap.h"
#include "Enums/Action.h"
#include "Tests/Testing.h"

TEST(HelpMapTest, GetHelpString) {
    HelpMap hm;
    const auto test_act = [&](Action act, const std::string &s){
        return(hm.GetHelpString(act).starts_with(s));
    };

    EXPECT_TRUE(test_act(Action::kQuit, "Exit the application"));
    EXPECT_TRUE(test_act(Action::kCopy, "Copy all selected models"));
    EXPECT_TRUE(test_act(Action::kToggleRightRadialMenu,
                "TOGGLE: Show or hide the right radial menu"));
}
