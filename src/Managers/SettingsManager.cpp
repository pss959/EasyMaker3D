#include "Managers/SettingsManager.h"

#include <fstream>

#include "Items/Settings.h"
#include "Parser/Exception.h"
#include "Parser/Parser.h"
#include "Parser/Writer.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/KLog.h"
#include "Util/Tuning.h"

SettingsManager::SettingsManager() {
    settings_ = Settings::CreateDefault();
}

bool SettingsManager::SetPath(const FilePath &path, bool save_on_set) {
    path_        = path;
    save_on_set_ = save_on_set;

    load_error_.clear();
    return path.Exists() ? LoadSettings_(path, load_error_) : false;
}

void SettingsManager::SetSettings(const Settings &new_settings) {
    settings_->CopyFrom(new_settings);
    if (path_ && save_on_set_)
        SaveSettings_(path_);
    if (change_func_)
        change_func_(new_settings);
}

bool SettingsManager::LoadSettings_(const FilePath &path, Str &error) {
    KLOG('f', "Reading settings from \"" << path.ToString() << "\"");
    SettingsPtr new_settings;
    error.clear();
    try {
        Parser::Parser parser;
        auto obj = parser.ParseFile(path);
        ASSERT(obj);
        new_settings = std::dynamic_pointer_cast<Settings>(obj);
        if (new_settings) {
            // Create a copy of the new settings; this starts with a default
            // instance, ensuring that all fields have a reasonable value.
            new_settings = Settings::CreateCopy(*new_settings);
        }
        else {
            error = "Got " + obj->GetTypeName() + " instead of Settings\n";
            return false;
        }
    }
    catch (const Parser::Exception &ex) {
        error = ex.what();
        return false;
    }
    settings_ = new_settings;
    return true;
}

void SettingsManager::SaveSettings_(const FilePath &path) {
    // Create directories if necessary.
    const auto dir_path = path.GetParentDirectory();
    if (dir_path.CreateDirectories()) {
        std::ofstream out(path.ToNativeString());
        if (out) {
            KLOG('f', "Writing settings to \"" << path.ToString() << "\"");
            Parser::Writer writer(out);
            writer.WriteObject(*settings_);
        }
    }
}
