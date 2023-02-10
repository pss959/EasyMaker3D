#include "Panels/SettingsPanel.h"

#include <string>

#include "Agents/SettingsAgent.h"
#include "Items/Settings.h"
#include "Items/UnitConversion.h"
#include "Panels/FilePanel.h"
#include "Panes/DropdownPane.h"
#include "Panes/SliderPane.h"
#include "Panes/TextInputPane.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/General.h"

void SettingsPanel::InitInterface() {
    InitDirectories_();
    InitTooltipSlider_();
    InitBuildVolume_();
    InitConversion_();

    // Shorthand for default and current buttons.
    auto def_cur = [&](const std::string &name){
        AddButtonFunc("Default" + name, [&, name](){ SetToDefault_(name); });
        AddButtonFunc("Current" + name, [&, name](){ SetToCurrent_(name); });
    };
    def_cur("SessionDir");
    def_cur("ExportDir");
    def_cur("ImportDir");
    def_cur("TooltipDelay");
    def_cur("BuildVolumeSize");
    def_cur("ExportConversion");
    def_cur("ImportConversion");

    // Set up all other button responses.
    AddButtonFunc("ChooseSessionDir", [&](){ OpenFilePanel_("SessionDir"); });
    AddButtonFunc("ChooseExportDir",  [&](){ OpenFilePanel_("ExportDir");  });
    AddButtonFunc("ChooseImportDir",  [&](){ OpenFilePanel_("ImportDir");  });
    AddButtonFunc("EditRadialMenus",  [&](){ OpenRadialMenuPanel_(); });
    AddButtonFunc("Cancel", [&](){ Close("Cancel"); });
    AddButtonFunc("Accept", [&](){ AcceptSettings_(); });
}

void SettingsPanel::UpdateInterface() {
    auto &settings = GetSettings();

    // Do not cause updates while this is happening.
    ignore_button_updates_ = true;

    UpdateFromSettings_(settings, "SessionDir");
    UpdateFromSettings_(settings, "ExportDir");
    UpdateFromSettings_(settings, "ImportDir");
    UpdateFromSettings_(settings, "TooltipDelay");
    UpdateFromSettings_(settings, "BuildVolumeSize");
    UpdateFromSettings_(settings, "ExportConversion");
    UpdateFromSettings_(settings, "ImportConversion");

    // Save default settings for comparisons.
    default_settings_ = Settings::CreateDefault();

    // Now update buttons.
    ignore_button_updates_ = false;
    EnableDefaultAndCurrentButtons_();
}

void SettingsPanel::InitDirectories_() {
    auto &root_pane = GetPane();
    session_pane_ = root_pane->FindTypedPane<TextInputPane>("SessionDir");
    export_pane_  = root_pane->FindTypedPane<TextInputPane>("ExportDir");
    import_pane_  = root_pane->FindTypedPane<TextInputPane>("ImportDir");

    // Set up directory validation.
    auto validator = [&](const std::string &s){
        EnableDefaultAndCurrentButtons_();
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
    tooltip_delay_slider_pane_->GetValueChanged().AddObserver(
        this, [&](float){ EnableDefaultAndCurrentButtons_(); });
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
        EnableDefaultAndCurrentButtons_();
        float size;
        return ParseSize_(s, size);
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
    auto upd = [&](const std::string &){ EnableDefaultAndCurrentButtons_(); };
    export_from_pane_->GetChoiceChanged().AddObserver(this, upd);
    export_to_pane_->GetChoiceChanged().AddObserver(this, upd);
    import_from_pane_->GetChoiceChanged().AddObserver(this, upd);
    import_to_pane_->GetChoiceChanged().AddObserver(this, upd);
}

void SettingsPanel::SetToDefault_(const std::string &name) {
    UpdateFromSettings_(*default_settings_, name);
    EnableDefaultAndCurrentButtons_();
}

void SettingsPanel::SetToCurrent_(const std::string &name) {
    UpdateFromSettings_(GetSettings(), name);
    EnableDefaultAndCurrentButtons_();
}

void SettingsPanel::EnableDefaultAndCurrentButtons_() {
    if (ignore_button_updates_)
        return;

    const Settings &ds = *default_settings_;
    const Settings &cs = GetSettings();

    auto update_dir = [&](const std::string &name,
                          const TextInputPane &pane, const FilePath &path){
        EnableButton(name, pane.GetText() != path.ToString());
    };
    update_dir("DefaultSessionDir", *session_pane_, ds.GetSessionDirectory());
    update_dir("CurrentSessionDir", *session_pane_, cs.GetSessionDirectory());
    update_dir("DefaultExportDir",  *export_pane_,  ds.GetExportDirectory());
    update_dir("CurrentExportDir",  *export_pane_,  cs.GetExportDirectory());
    update_dir("DefaultImportDir",  *import_pane_,  ds.GetImportDirectory());
    update_dir("CurrentImportDir",  *import_pane_,  cs.GetImportDirectory());

    EnableButton(
        "DefaultTooltipDelay",
        tooltip_delay_slider_pane_->GetValue() != ds.GetTooltipDelay());
    EnableButton(
        "CurrentTooltipDelay",
        tooltip_delay_slider_pane_->GetValue() != cs.GetTooltipDelay());

    auto bvs_differs = [&](const Settings &settings){
        const Vector3f &bv_size = settings.GetBuildVolumeSize();
        return
            build_volume_panes_[0]->GetText() != Util::ToString(bv_size[0]) ||
            build_volume_panes_[1]->GetText() != Util::ToString(bv_size[1]) ||
            build_volume_panes_[2]->GetText() != Util::ToString(bv_size[2]);
    };
    EnableButton("DefaultBuildVolumeSize", bvs_differs(ds));
    EnableButton("CurrentBuildVolumeSize", bvs_differs(cs));

    auto conv_differs = [](const UnitConversion &conv,
                           const DropdownPane &from_pane,
                           const DropdownPane &to_pane){
        return
            from_pane.GetChoiceIndex() != Util::EnumInt(conv.GetFromUnits()) ||
            to_pane.GetChoiceIndex()   != Util::EnumInt(conv.GetToUnits());
    };
    EnableButton("DefaultExportConversion",
                 conv_differs(ds.GetExportUnitsConversion(),
                              *export_from_pane_, *export_to_pane_));
    EnableButton("CurrentExportConversion",
                 conv_differs(cs.GetExportUnitsConversion(),
                              *export_from_pane_, *export_to_pane_));
    EnableButton("DefaultImportConversion",
                 conv_differs(ds.GetImportUnitsConversion(),
                              *import_from_pane_, *import_to_pane_));
    EnableButton("CurrentImportConversion",
                 conv_differs(cs.GetImportUnitsConversion(),
                              *import_from_pane_, *import_to_pane_));
}

void SettingsPanel::UpdateFromSettings_(const Settings &settings,
                                        const std::string &name) {
    auto set_dd = [](DropdownPane &pane, UnitConversion::Units unit){
        pane.SetChoice(Util::EnumInt(unit));
    };

    if (name == "SessionDir") {
        session_pane_->SetInitialText(
            settings.GetSessionDirectory().ToString());
    }
    else if (name == "ExportDir") {
        export_pane_->SetInitialText(settings.GetExportDirectory().ToString());
    }
    else if (name == "ImportDir") {
        import_pane_->SetInitialText(settings.GetImportDirectory().ToString());
    }
    else if (name == "TooltipDelay") {
        tooltip_delay_slider_pane_->SetValue(settings.GetTooltipDelay());
    }
    else if (name == "BuildVolumeSize") {
        const Vector3f &bv_size = settings.GetBuildVolumeSize();
        for (int i = 0; i < 3; ++i)
            build_volume_panes_[i]->SetInitialText(Util::ToString(bv_size[i]));
    }
    else if (name == "ExportConversion") {
        const auto &export_conv = settings.GetExportUnitsConversion();
        set_dd(*export_from_pane_, export_conv.GetFromUnits());
        set_dd(*export_to_pane_,   export_conv.GetToUnits());
    }
    else if (name == "ImportConversion") {
        const auto &import_conv = settings.GetImportUnitsConversion();
        set_dd(*import_from_pane_, import_conv.GetFromUnits());
        set_dd(*import_to_pane_,   import_conv.GetToUnits());
    }
}

void SettingsPanel::OpenFilePanel_(const std::string &item_name) {
    auto &helper = *GetContext().panel_helper;
    auto fp = helper.GetTypedPanel<FilePanel>("FilePanel");
    InitFilePanel_(*fp, item_name);

    auto result_func = [&, fp, item_name](const std::string &result){
        if (result == "Accept")
            AcceptFileItem_(item_name, fp->GetPath());
    };
    helper.PushPanel(fp, result_func);
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
    auto &helper = *GetContext().panel_helper;
    helper.PushPanel(helper.GetPanel("RadialMenuPanel"), nullptr);
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
        float size;
        if (ParseSize_(build_volume_panes_[i]->GetText(), size))
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

    GetContext().settings_agent->SetSettings(*new_settings);

    Close("Accept");
}

bool SettingsPanel::ParseSize_(const std::string &s, float &size) {
    size_t pos;
    try {
        size = std::stof(s, &pos);
        // Make sure the entire string was parsed.
        return pos == s.size();
    }
    catch (std::exception &) {
        return false;
    }
}
