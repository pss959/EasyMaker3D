#include "Panels/SettingsPanel.h"

#include "Managers/SettingsManager.h"
#include "Panels/FilePanel.h"
#include "Panes/TextInputPane.h"
#include "SG/Search.h"
#include "Settings.h"
#include "Util/Assert.h"

void SettingsPanel::InitInterface() {
    AddButtonFunc("ChooseSessionDir", [&](){ OpenFilePanel_("SessionDir"); });
    AddButtonFunc("ChooseExportDir",  [&](){ OpenFilePanel_("ExportDir");  });

    AddButtonFunc("Cancel", [&](){ Close("Cancel"); });
    AddButtonFunc("Accept", [&](){ AcceptSettings_(); });
}

void SettingsPanel::UpdateInterface() {
    const auto &settings = GetContext().settings_manager->GetSettings();

    auto init_input = [&](const std::string &name, const std::string &text){
        auto input = SG::FindTypedNodeUnderNode<TextInputPane>(*this, name);
        input->SetValidationFunc([](const std::string &s){
            return Util::FilePath(s).IsDirectory();
        });
        input->SetInitialText(text);
    };

    init_input("SessionDir", settings.session_directory.ToString());
    init_input("ExportDir",  settings.export_directory.ToString());

    // XXXX More...
}

void SettingsPanel::OpenFilePanel_(const std::string &item_name) {
    auto init = [&](Panel &p){
        ASSERT(p.GetTypeName() == "FilePanel");
        InitFilePanel_(static_cast<FilePanel &>(p), item_name);
    };
    auto result = [&](Panel &p, const std::string &res){
        if (res == "Accept") {
            ASSERT(p.GetTypeName() == "FilePanel");
            FilePanel &fp = static_cast<FilePanel &>(p);
            AcceptFileItem_(item_name, fp.GetPath());
        }
    };
    GetContext().panel_helper->Replace("FilePanel", init, result);
}

void SettingsPanel::InitFilePanel_(FilePanel &file_panel,
                                   const std::string &item_name) {
    const std::string file_desc =
        item_name == "SessionDir" ? "Session" :
        item_name == "ExportDir"  ? "STL Export" : "STL Import";

    auto input = SG::FindTypedNodeUnderNode<TextInputPane>(*this, item_name);

    file_panel.Reset();
    file_panel.SetTitle("Select a directory for " + file_desc + " files");
    file_panel.SetInitialPath(Util::FilePath(input->GetText()));
}

void SettingsPanel::AcceptFileItem_(const std::string &item_name,
                                    const Util::FilePath &path) {
    auto input = SG::FindTypedNodeUnderNode<TextInputPane>(*this, item_name);
    input->SetInitialText(path.ToString());

}

void SettingsPanel::AcceptSettings_() {
    std::cerr << "XXXX AcceptSettings_\n";
    // XXXX Update settings in SettingsManager.
    Close("Accept");
}
