#include "Panels/SettingsPanel.h"

#include "Managers/SettingsManager.h"
#include "Panes/TextInputPane.h"
#include "SG/Search.h"
#include "Settings.h"
#include "Util/FilePath.h"

void SettingsPanel::InitInterface() {
    //AddButtonFunc("Help",     [this](){ OpenHelp_();         });
}

void SettingsPanel::UpdateInterface() {
    const auto &settings = GetContext().settings_manager->GetSettings();
    auto input = SG::FindTypedNodeUnderNode<TextInputPane>(*this, "SessionDir");
    input->SetInitialText(settings.session_directory);
    // XXXX More...
}
