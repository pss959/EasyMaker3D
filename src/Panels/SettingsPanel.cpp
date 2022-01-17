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
    const auto &settings = GetSettings();

    auto init_input = [&](const std::string &name, const std::string &text){
        auto input = SG::FindTypedNodeUnderNode<TextInputPane>(*this, name);
        input->SetValidationFunc([](const std::string &s){
            return FilePath(s).IsDirectory();
        });
        input->SetInitialText(text);
    };

    init_input("SessionDir", settings.GetSessionDirectory().ToString());
    init_input("ExportDir",  settings.GetExportDirectory().ToString());

    // XXXX More...
}

void SettingsPanel::OpenFilePanel_(const std::string &item_name) {
    auto init = [&](Panel &p){
        ASSERT(p.GetTypeName() == "FilePanel");
        InitFilePanel_(static_cast<FilePanel &>(p), item_name);
    };
    auto result = [&, item_name](Panel &p, const std::string &res){
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
    file_panel.SetInitialPath(FilePath(input->GetText()));
}

void SettingsPanel::AcceptFileItem_(const std::string &item_name,
                                    const FilePath &path) {
    auto input = SG::FindTypedNodeUnderNode<TextInputPane>(*this, item_name);
    input->SetInitialText(path.ToString());
}

void SettingsPanel::AcceptSettings_() {
    std::cerr << "XXXX AcceptSettings_\n";

    // Copy the current settings.
    SettingsPtr new_settings = Settings::CreateDefault();
    new_settings->CopyFrom(GetSettings());

    // Update from UI fields.
    auto get_input = [&](const std::string &name){
        auto input = SG::FindTypedNodeUnderNode<TextInputPane>(*this, name);
        return FilePath(input->GetText());
    };

    new_settings->SetSessionDirectory(get_input("SessionDir"));
    new_settings->SetExportDirectory(get_input("ExportDir"));
    /* XXXX
    settings.import_directory  = get_input("ImportDir");
    settings.tooltipDelay     = _tooltipDelaySlider.GetValue();
    settings.buildVolumeSize  = GetBuildVolumeSize();
    settings.importConversionInfo = GetConversionInfo(_importDropDowns);
    settings.exportConversionInfo = GetConversionInfo(_exportDropDowns);
    */
    GetContext().settings_manager->SetSettings(*new_settings);

    Close("Accept");
}
