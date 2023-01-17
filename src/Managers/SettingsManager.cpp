#include "Managers/SettingsManager.h"

#include <fstream>

#include "Base/Tuning.h"
#include "Items/Settings.h"
#include "Parser/Exception.h"
#include "Parser/Parser.h"
#include "Parser/Writer.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/General.h"
#include "Util/KLog.h"

SettingsManager::SettingsManager() {
    // See if there is a file with settings.
    const auto path = GetSettingsFilePath_(FilePath::GetSettingsDirPath());
    if (path.Exists())
        settings_ = ReadSettings_(path);
    if (! settings_)
        settings_ = Settings::CreateDefault();
}

void SettingsManager::SetSettings(const Settings &new_settings) {
    settings_->CopyFrom(new_settings);
    if (write_settings_)
        WriteSettings_();
    if (change_func_)
        change_func_(new_settings);
}

bool SettingsManager::ReplaceSettings(const FilePath &path) {
    if (! path.Exists()) {
        std::cerr << "*** Unable to read settings from '"
                  << path.ToString() << "': no such file\n";
        return false;
    }
    if (SettingsPtr new_settings = ReadSettings_(path)) {
        settings_ = new_settings;
        return true;
    }
    return false;
}

SettingsPtr SettingsManager::ReadSettings_(const FilePath &path) {
    KLOG('f', "Reading settings from \"" << path.ToString() << "\"");
    SettingsPtr settings;
    try {
        Parser::Parser parser;
        auto obj = parser.ParseFile(path);
        ASSERT(obj);
        settings = Util::CastToDerived<Settings>(obj);
        if (! settings)
            std::cerr << "*** Unable to read settings from '"
                      << path.ToString() << "':\n Got " << obj->GetDesc()
                      << " instead of Settings\n";
    }
    catch (const Parser::Exception &ex) {
        std::cerr << "*** Unable to read settings from '"
                  << path.ToString() << "':\n" << ex.what() << "\n";
    }
    return settings;
}

void SettingsManager::WriteSettings_() {
    // Make sure the directory path exists.
    const auto dir_path = FilePath::GetSettingsDirPath();
    if (! dir_path.CreateDirectories()) {
        std::cerr << "*** Unable to create settings directory '"
                  << dir_path.ToString() << "'\n";
        return;
    }

    const auto path = GetSettingsFilePath_(dir_path);
    KLOG('f', "Writing settings to \"" << path.ToString() << "\"");
    std::ofstream out(path.ToNativeString());
    if (out.fail()) {
        std::cerr << "*** Unable to write settings to '"
                  << path.ToString() << "'\n";
        return;
    }

    Parser::Writer writer(out);
    writer.WriteObject(*settings_);
}

FilePath SettingsManager::GetSettingsFilePath_(const FilePath &dir_path) {
    return FilePath::Join(dir_path,
                          std::string("settings") + TK::kDataFileSuffix);
}
