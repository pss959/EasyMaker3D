#include "Panels/SettingsPanel.h"

#include <string>

#include "Managers/SettingsManager.h"
#include "Panels/FilePanel.h"
#include "Panes/DropdownPane.h"
#include "Panes/SliderPane.h"
#include "Panes/TextInputPane.h"
#include "SG/Search.h"
#include "Settings.h"
#include "UnitConversion.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/General.h"

void SettingsPanel::InitInterface() {
    InitDirectories_();
    InitTooltipSlider_();
    InitBuildVolume_();
    InitConversion_();

    // Set up all button responses.
    AddButtonFunc("ChooseSessionDir", [&](){ OpenFilePanel_("SessionDir"); });
    AddButtonFunc("ChooseExportDir",  [&](){ OpenFilePanel_("ExportDir");  });
    AddButtonFunc("ChooseImportDir",  [&](){ OpenFilePanel_("ImportDir");  });
    AddButtonFunc("EditRadialMenus",  [&](){ OpenRadialMenuPanel_(); });
    AddButtonFunc("Cancel", [&](){ Close("Cancel"); });
    AddButtonFunc("Accept", [&](){ AcceptSettings_(); });
}

void SettingsPanel::UpdateInterface() {
    const auto &settings = GetSettings();

    // Set current directory inputs.
    session_pane_->SetInitialText(settings.GetSessionDirectory().ToString());
    export_pane_->SetInitialText(settings.GetExportDirectory().ToString());
    import_pane_->SetInitialText(settings.GetImportDirectory().ToString());

    // Set current tooltip delay.
    tooltip_delay_slider_pane_->SetValue(settings.GetTooltipDelay());

    // Set current build volume dimensions.
    const Vector3f &bv_size = settings.GetBuildVolumeSize();
    for (int i = 0; i < 3; ++i)
        build_volume_panes_[i]->SetInitialText(Util::ToString(bv_size[i]));

    // Set current unit conversion dropdowns.
    auto set_choice = [](DropdownPane &pane, UnitConversion::Units unit){
        pane.SetChoice(Util::EnumInt(unit));
    };
    const UnitConversion &export_conv = settings.GetExportUnitsConversion();
    const UnitConversion &import_conv = settings.GetImportUnitsConversion();
    set_choice(*export_from_pane_, export_conv.GetFromUnits());
    set_choice(*export_to_pane_,   export_conv.GetToUnits());
    set_choice(*import_from_pane_, import_conv.GetFromUnits());
    set_choice(*import_to_pane_,   import_conv.GetToUnits());
}

void SettingsPanel::InitDirectories_() {
    auto &root_pane = GetPane();
    session_pane_ = root_pane->FindTypedPane<TextInputPane>("SessionDir");
    export_pane_  = root_pane->FindTypedPane<TextInputPane>("ExportDir");
    import_pane_  = root_pane->FindTypedPane<TextInputPane>("ImportDir");

    // Set up directory validation.
    auto validator = [](const std::string &s){
        return FilePath(s).IsDirectory();
    };
    session_pane_->SetValidationFunc(validator);
    export_pane_->SetValidationFunc(validator);
    import_pane_->SetValidationFunc(validator);
}

void SettingsPanel::InitTooltipSlider_() {
    auto &root_pane = GetPane();
    auto pane = root_pane->FindTypedPane<ContainerPane>("TooltipDelay");
    tooltip_delay_slider_pane_ = pane->FindTypedPane<SliderPane>("Slider");
}

void SettingsPanel::InitBuildVolume_() {
    auto &root_pane = GetPane();
    build_volume_panes_[0] =
        root_pane->FindTypedPane<TextInputPane>("BuildVolumeWidth");
    build_volume_panes_[1] =
        root_pane->FindTypedPane<TextInputPane>("BuildVolumeDepth");
    build_volume_panes_[2] =
        root_pane->FindTypedPane<TextInputPane>("BuildVolumeHeight");

    // Set up size validation.
    auto validator = [&](const std::string &s){
        size_t pos;
        static_cast<void>(std::stof(s, &pos));
        return pos == s.size();
    };
    for (int i = 0; i < 3; ++i)
        build_volume_panes_[i]->SetValidationFunc(validator);
}

void SettingsPanel::InitConversion_() {
    auto &root_pane = GetPane();
    export_from_pane_ = root_pane->FindTypedPane<DropdownPane>("ExportFrom");
    export_to_pane_   = root_pane->FindTypedPane<DropdownPane>("ExportTo");
    import_from_pane_ = root_pane->FindTypedPane<DropdownPane>("ImportFrom");
    import_to_pane_   = root_pane->FindTypedPane<DropdownPane>("ImportTo");

    const std::vector<std::string> units =
        Util::ConvertVector<std::string, UnitConversion::Units>(
            Util::EnumValues<UnitConversion::Units>(),
            [](const UnitConversion::Units &u){ return Util::EnumToWords(u); });
    export_from_pane_->SetChoices(units, 0);
    export_to_pane_->SetChoices(units, 0);
    import_from_pane_->SetChoices(units, 0);
    import_to_pane_->SetChoices(units, 0);
}

void SettingsPanel::OpenFilePanel_(const std::string &item_name) {
    auto init = [&](const PanelPtr &p){
        ASSERT(p->GetTypeName() == "FilePanel");
        InitFilePanel_(*Util::CastToDerived<FilePanel>(p), item_name);
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

void SettingsPanel::OpenRadialMenuPanel_() {
    GetContext().panel_helper->Replace("RadialMenuPanel", nullptr, nullptr);
}

void SettingsPanel::AcceptSettings_() {
    // Copy the current settings.
    SettingsPtr new_settings = Settings::CreateDefault();
    new_settings->CopyFrom(GetSettings());

    // Access from directory input.
    auto get_dir = [&](const std::string &name){
        auto input = SG::FindTypedNodeUnderNode<TextInputPane>(*this, name);
        return FilePath(input->GetText());
    };

    // Access valid build volume sizes. Use current settings if new ones are
    // not valid.
    Vector3f bv_size = new_settings->GetBuildVolumeSize();
    for (int i = 0; i < 3; ++i) {
        size_t pos;
        const std::string &s = build_volume_panes_[i]->GetText();
        const float size = std::stof(s, &pos);
        if (pos == s.size())
            bv_size[i] = size;
    }

    // Access unit conversion values.
    auto get_unit = [](const DropdownPane &dd){
        return Util::EnumValues<UnitConversion::Units>()[dd.GetChoiceIndex()];
    };
    UnitConversionPtr import_conv =
        UnitConversion::CreateWithUnits(get_unit(*import_from_pane_),
                                        get_unit(*import_to_pane_));
    UnitConversionPtr export_conv =
        UnitConversion::CreateWithUnits(get_unit(*export_from_pane_),
                                        get_unit(*export_to_pane_));

    new_settings->SetSessionDirectory(get_dir("SessionDir"));
    new_settings->SetExportDirectory(get_dir("ExportDir"));
    new_settings->SetImportDirectory(get_dir("ImportDir"));
    new_settings->SetTooltipDelay(tooltip_delay_slider_pane_->GetValue());
    new_settings->SetBuildVolumeSize(bv_size);
    new_settings->SetImportUnitsConversion(*import_conv);
    new_settings->SetExportUnitsConversion(*export_conv);

    GetContext().settings_manager->SetSettings(*new_settings);

    Close("Accept");
}
