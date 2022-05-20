﻿#include "Managers/SettingsManager.h"

#include <fstream>

#include "Items/Settings.h"
#include "Parser/Exception.h"
#include "Parser/Parser.h"
#include "Parser/Writer.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/General.h"

SettingsManager::SettingsManager() {
    // See if there is a file with settings.
    const FilePath path = GetSettingsFilePath_();
    if (path.Exists())
        settings_ = ReadSettings_(path);
    if (! settings_)
        settings_ = Settings::CreateDefault();
}

void SettingsManager::SetSettings(const Settings &new_settings) {
    settings_->CopyFrom(new_settings);
    WriteSettings_(GetSettingsFilePath_());
    if (change_func_)
        change_func_(new_settings);
}

SettingsPtr SettingsManager::ReadSettings_(const FilePath &path) {
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

void SettingsManager::WriteSettings_(const FilePath &path) {
    std::ofstream out(path.ToNativeString());
    if (out.fail()) {
        std::cerr << "*** Unable to write settings to '"
                  << path.ToString() << "'\n";
        return;
    }

    Parser::Writer writer(out);
    writer.WriteObject(*settings_);
}

FilePath SettingsManager::GetSettingsFilePath_() {
    return FilePath("settings.mvr");
}
