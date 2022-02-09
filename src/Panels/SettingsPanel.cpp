#include "Panels/SettingsPanel.h"

#include "Managers/SettingsManager.h"
#include "Panels/FilePanel.h"
#include "SG/Search.h"
#include "Settings.h"
#include "UnitConversion.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/General.h"

void SettingsPanel::InitInterface() {
    AddButtonFunc("ChooseSessionDir", [&](){ OpenFilePanel_("SessionDir"); });
    AddButtonFunc("ChooseExportDir",  [&](){ OpenFilePanel_("ExportDir");  });

    AddButtonFunc("Cancel", [&](){ Close("Cancel"); });
    AddButtonFunc("Accept", [&](){ AcceptSettings_(); });
}

void SettingsPanel::UpdateInterface() {
    const auto &settings = GetSettings();

    // Find panes.
    auto &root_pane = GetPane();
    session_pane_     = root_pane->FindTypedPane<TextInputPane>("SessionDir");
    export_pane_      = root_pane->FindTypedPane<TextInputPane>("ExportDir");
    import_pane_      = root_pane->FindTypedPane<TextInputPane>("ImportDir");
    export_from_pane_ = root_pane->FindTypedPane<DropdownPane>("ExportFrom");
    export_to_pane_   = root_pane->FindTypedPane<DropdownPane>("ExportTo");
    import_from_pane_ = root_pane->FindTypedPane<DropdownPane>("ImportFrom");
    import_to_pane_   = root_pane->FindTypedPane<DropdownPane>("ImportTo");

    // Initialize directory inputs.
    auto init_input = [&](TextInputPane &input, const FilePath &path){
        input.SetValidationFunc([](const std::string &s){
            return FilePath(s).IsDirectory();
        });
        input.SetInitialText(path.ToString());
    };
    init_input(*session_pane_, settings.GetSessionDirectory());
    init_input(*export_pane_,  settings.GetExportDirectory());
    init_input(*import_pane_,  settings.GetImportDirectory());

    // Initialize unit conversion dropdowns.
    const std::vector<std::string> units =
        Util::ConvertVector<std::string, UnitConversion::Units>(
            Util::EnumValues<UnitConversion::Units>(),
            [](const UnitConversion::Units &u){ return Util::EnumToWords(u); });
    auto init_dropdown = [&](DropdownPane &dd){
        dd.SetChoices(units, 0);
    };
    init_dropdown(*export_from_pane_);
    init_dropdown(*export_to_pane_);
    init_dropdown(*import_from_pane_);
    init_dropdown(*import_to_pane_);

    auto slider_pane = root_pane->FindTypedPane<ContainerPane>("TooltipDelay");
    tooltip_delay_slider_pane_ =
        slider_pane->FindTypedPane<SliderPane>("Slider");
    tooltip_delay_slider_pane_->SetValue(settings.GetTooltipDelay());

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
    new_settings->SetImportDirectory(get_input("ImportDir"));
    new_settings->SetTooltipDelay(tooltip_delay_slider_pane_->GetValue());

    /* XXXX
    settings.buildVolumeSize  = GetBuildVolumeSize();
    settings.importConversionInfo = GetConversionInfo(_importDropDowns);
    settings.exportConversionInfo = GetConversionInfo(_exportDropDowns);
    */
    GetContext().settings_manager->SetSettings(*new_settings);

    Close("Accept");
}
