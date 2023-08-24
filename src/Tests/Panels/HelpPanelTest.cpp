#include "Panels/HelpPanel.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"
#include "Util/Tuning.h"
#include "Util/URL.h"

class HelpPanelTest : public PanelTestBase {};

TEST_F(HelpPanelTest, Defaults) {
    auto panel = ReadRealPanel<HelpPanel>("HelpPanel");
    EXPECT_NULL(panel->GetFocusedPane());
}

TEST_F(HelpPanelTest, Show) {
    auto panel = ReadRealPanel<HelpPanel>("HelpPanel");
    EXPECT_FALSE(panel->IsShown());
    panel->SetIsShown(true);
    EXPECT_TRUE(panel->IsShown());
    EXPECT_EQ(FindPane(*panel, "Done"), panel->GetFocusedPane());
}

TEST_F(HelpPanelTest, OpenURLs) {
    Str last_url;
    Util::SetOpenURLFunc([&](const Str &url){ last_url = url; });

    auto panel = ReadRealPanel<HelpPanel>("HelpPanel");
    panel->SetIsShown(true);

    auto doc_url = [](const Str &page){
        return TK::kPublicDocBaseURL + "/" + TK::kVersionString + "/" + page;
    };

    panel->SetIsShown(true);
    EXPECT_TRUE(panel->IsShown());

    ClickButtonPane(*panel, "UserGuide");
    EXPECT_EQ(doc_url("UserGuide"), last_url);

    ClickButtonPane(*panel, "CheatSheet");
    EXPECT_EQ(doc_url("CheatSheet"), last_url);

    ClickButtonPane(*panel, "Issue");
    EXPECT_EQ(TK::kGithubURL + "/issues", last_url);

    ClickButtonPane(*panel, "Done");
    EXPECT_FALSE(panel->IsShown());
    EXPECT_EQ("Done", GetCloseResult());
}
