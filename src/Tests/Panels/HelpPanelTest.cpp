#include "Panels/HelpPanel.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"
#include "Util/Tuning.h"
#include "Util/URL.h"

/// \ingroup Tests
class HelpPanelTest : public PanelTestBase {
  protected:
    HelpPanelPtr panel;
    HelpPanelTest() { panel = InitPanel<HelpPanel>("HelpPanel"); }
};

TEST_F(HelpPanelTest, Defaults) {
    EXPECT_NULL(panel->GetFocusedPane());
}

TEST_F(HelpPanelTest, Show) {
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());
    EXPECT_EQ(FindPane("Done"), panel->GetFocusedPane());
}

TEST_F(HelpPanelTest, OpenURLs) {
    Str last_url;
    Util::SetOpenURLFunc([&](const Str &url){ last_url = url; });

    auto doc_url = [](const Str &page){
        return TK::kPublicDocBaseURL + "/" + TK::kVersionString + "/" + page;
    };

    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());

    ClickButtonPane("UserGuide");
    EXPECT_EQ(doc_url("UserGuide"), last_url);

    ClickButtonPane("CheatSheet");
    EXPECT_EQ(doc_url("CheatSheet"), last_url);

    ClickButtonPane("Issue");
    EXPECT_EQ(TK::kGithubURL + "/issues", last_url);

    ClickButtonPane("Done");
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    EXPECT_EQ("Done", GetCloseResult());
}
