#include "Panels/SettingsPanel.h"

#include "Managers/SettingsManager.h"
#include "Panels/FilePanel.h"
#include "Panes/TextInputPane.h"
#include "SG/Search.h"
#include "Settings.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"

void SettingsPanel::InitInterface() {
    AddButtonFunc("ChooseSessionDir",
                  [this](){ OpenFilePanel_("SessionDir"); });
    AddButtonFunc("ChooseExportDir",
                  [this](){ OpenFilePanel_("ExportDir"); });

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

void SettingsPanel::OpenFilePanel_(const std::string &name) {
    // Save the name so InitReplacementPanel() can operate.
    file_panel_target_ = name;
    Close(CloseReason::kReplaceAndRestore, "FilePanel");
}

void SettingsPanel::InitReplacementPanel(Panel &new_panel) {
    ASSERT(new_panel.GetTypeName() == "FilePanel");
    FilePanel &file_panel = static_cast<FilePanel &>(new_panel);
    ASSERT(! file_panel_target_.empty());

    std::string file_desc =
        file_panel_target_ == "SessionDir" ? "Session" :
        file_panel_target_ == "ExportDir"  ? "STL Export" : "STL Import";

    auto input = SG::FindTypedNodeUnderNode<TextInputPane>(
        *this, file_panel_target_);

    file_panel.Reset();
    file_panel.SetTitle("Select a directory for " + file_desc + " files");
    file_panel.SetInitialPath(Util::FilePath(input->GetText()));
}

void SettingsPanel::AcceptSettings_() {
    std::cerr << "XXXX AcceptSettings_\n";
}
