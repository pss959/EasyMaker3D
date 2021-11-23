#include "Panels/SettingsPanel.h"

#include "Managers/SettingsManager.h"
#include "Panes/TextInputPane.h"
#include "SG/Search.h"
#include "Settings.h"
#include "Util/FilePath.h"

void SettingsPanel::InitInterface() {
    AddButtonFunc("ChooseSessionDir",
                  [this](){ OpenFileBrowser_("SessionDir"); });
    AddButtonFunc("ChooseExportDir",
                  [this](){ OpenFileBrowser_("ExportDir"); });

    AddButtonFunc("Cancel", [this](){ Close(CloseReason::kDone, "Cancel"); });
    AddButtonFunc("Accept", [this](){ AcceptSettings_(); });
}

void SettingsPanel::UpdateInterface() {
    const auto &settings = GetContext().settings_manager->GetSettings();

    auto init_input = [this](const std::string &name, const std::string &text){
        auto input = SG::FindTypedNodeUnderNode<TextInputPane>(*this, name);
        input->SetValidationFunc([](const std::string &s){
            return Util::FilePath(s).IsDirectory();
        });
        input->SetInitialText(text);
    };

    init_input("SessionDir", settings.session_directory);
    init_input("ExportDir",  settings.export_directory);

    // XXXX More...
}

void SettingsPanel::OpenFileBrowser_(const std::string &name) {
    std::cerr << "XXXX Open File browser for " << name << "\n";
}

void SettingsPanel::AcceptSettings_() {
    std::cerr << "XXXX AcceptSettings_\n";
}
