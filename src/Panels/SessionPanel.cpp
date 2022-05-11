#include "Panels/SessionPanel.h"

#include "Managers/SessionManager.h"
#include "Managers/SettingsManager.h"
#include "Panels/FilePanel.h"
#include "Settings.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/String.h"

void SessionPanel::InitInterface() {
    AddButtonFunc("Help",     [this](){ OpenHelp_();         });
    AddButtonFunc("Settings", [this](){ OpenSettings_();     });
    AddButtonFunc("Continue", [this](){ ContinueSession_();  });
    AddButtonFunc("Load",     [this](){ LoadSession_();      });
    AddButtonFunc("New",      [this](){ StartNewSession_();  });
    AddButtonFunc("Save",     [this](){ SaveSession_(true);  });
    AddButtonFunc("SaveAs",   [this](){ SaveSession_(false); });
    AddButtonFunc("Export",   [this](){ ExportSelection_();  });
}

void SessionPanel::UpdateInterface() {
    auto &session_manager = *GetContext().session_manager;

    const FilePath &session_path = GetSettings().GetLastSessionPath();
    std::string continue_text;

    const bool have_session = session_manager.SessionStarted();
    const bool can_continue = have_session && ! session_path.IsDirectory();
    if (can_continue) {
        const bool is_current =
            session_manager.GetSessionPath() == session_path;
        continue_text = std::string("Continue ") +
            (is_current ? "current" : "previous") + " session " +
            session_path.GetFileName();
    }
    else {
        continue_text = "(No previous session)";
    }
    SetButtonText("Continue", continue_text);
    EnableButton("Continue", can_continue);
    EnableButton("Load",     true);
    EnableButton("Save",     have_session && session_manager.CanSaveSession());
    EnableButton("Export",   session_manager.CanExport());

    // Move the focus to a button that is enabled unless there is already a
    // focused pane.
    if (! GetFocusedPane())
        SetFocus(can_continue ? "Continue" : "New");
}

void SessionPanel::Close(const std::string &result) {
    // Canceling starts a new session if this is the initial SessionPanel.
    if (result == "Cancel" && ! GetContext().session_manager->SessionStarted())
        StartNewSession_();
    return Panel::Close(result);
}

void SessionPanel::OpenHelp_() {
    GetContext().panel_helper->Replace("HelpPanel", nullptr, nullptr);
}

void SessionPanel::OpenSettings_() {
    GetContext().panel_helper->Replace("SettingsPanel", nullptr, nullptr);
}

void SessionPanel::ContinueSession_() {
    // Do nothing if the session is already loaded.
    auto &session_manager = GetContext().session_manager;
    const auto &session_path = GetSettings().GetLastSessionPath();

    if (session_manager->GetSessionPath() == session_path) {
        Close("Done");
    }
    else {
        ASSERT(session_path.Exists());
        std::string error;
        if (session_manager->LoadSession(session_path, error))
            Close("Done");
        else
            DisplayMessage("Could not load session from '" +
                           session_path.ToString() + "':\n" + error, nullptr);
    }
}

void SessionPanel::LoadSession_() {
    auto init = [&](const PanelPtr &p){
        ASSERT(p->GetTypeName() == "FilePanel");
        FilePanel &fp = *Util::CastToDerived<FilePanel>(p);
        const auto &settings = GetSettings();
        fp.Reset();
        fp.SetTitle("Select a session file (.mvr) to load");
        fp.SetInitialPath(settings.GetSessionDirectory());
        fp.SetTargetType(FilePanel::TargetType::kExistingFile);
        fp.SetExtension(".mvr");
        fp.SetHighlightPath(settings.GetLastSessionPath(),
                            " [CURRENT SESSION]");
    };
    auto result = [&](Panel &p, const std::string &res){
        ASSERT(p.GetTypeName() == "FilePanel");
        FilePanel &fp = static_cast<FilePanel &>(p);
        if (res == "Accept")
            LoadSessionFromPath_(fp.GetPath());
    };
    GetContext().panel_helper->Replace("FilePanel", init, result);
}

void SessionPanel::StartNewSession_() {
    // If changes were made in the current session (and it can therefore be
    // saved), ask the user what to do.
    if (GetContext().session_manager->CanSaveSession()) {
        const std::string msg = "There are unsaved changes."
            " Do you really want to start a new session?";
        auto func = [&](const std::string &answer){
            if (answer == "Yes") {
                Close("Done");
                GetContext().session_manager->NewSession();
            }
        };
        AskQuestion(msg, func);
    }
    else {
        Close("Done");
        GetContext().session_manager->NewSession();
        SetLastSessionPath_(FilePath());
    }
}

void SessionPanel::SaveSession_(bool use_current_file) {
    if (use_current_file) {
        SaveSessionToPath_(GetSettings().GetLastSessionPath());
    }
    else {
        auto init = [&](const PanelPtr &p){
            ASSERT(p->GetTypeName() == "FilePanel");
            FilePanel &fp = *Util::CastToDerived<FilePanel>(p);
            const auto &settings = GetSettings();
            fp.Reset();
            fp.SetTitle("Enter a session file (.mvr) to save to");
            fp.SetInitialPath(settings.GetSessionDirectory());
            fp.SetTargetType(FilePanel::TargetType::kNewFile);
            fp.SetExtension(".mvr");
            fp.SetHighlightPath(settings.GetLastSessionPath(),
                                " [CURRENT SESSION]");
        };
        auto result = [&](Panel &p, const std::string &res){
            ASSERT(p.GetTypeName() == "FilePanel");
            FilePanel &fp = static_cast<FilePanel &>(p);
            if (res == "Accept")
                SaveSessionToPath_(fp.GetPath());
        };
        GetContext().panel_helper->Replace("FilePanel", init, result);
    }
}

void SessionPanel::ExportSelection_() {
    auto init = [&](const PanelPtr &p){
        ASSERT(p->GetTypeName() == "FilePanel");
        FilePanel &fp = *Util::CastToDerived<FilePanel>(p);
        fp.Reset();
        fp.SetTitle("Enter a file to export to");
        fp.SetInitialPath(GetInitialExportPath_());
        fp.SetTargetType(FilePanel::TargetType::kNewFile);
        fp.SetExtension(".stl");
        std::vector<std::string> formats;
        for (auto &format: Util::EnumValues<FileFormat>()) {
            if (format != FileFormat::kUnknown)
                formats.push_back(Util::EnumToWords(format));
        }
        fp.SetFileFormats(formats);
    };
    auto result = [&](Panel &p, const std::string &res){
        ASSERT(p.GetTypeName() == "FilePanel");
        FilePanel &fp = static_cast<FilePanel &>(p);
        if (res == "Accept") {
            FileFormat format = FileFormat::kUnknown;
            Util::EnumFromString(fp.GetFileFormat(), format);
            ASSERT(format != FileFormat::kUnknown);
            ExportToPath_(fp.GetPath(), format);
        }
    };
    GetContext().panel_helper->Replace("FilePanel", init, result);
}

FilePath SessionPanel::GetInitialExportPath_() {
    // If the current session file is named "Foo.mvr", assume the STL target
    // file is "Foo.stl". Otherwise, leave the name blank.
    const auto &settings = GetSettings();
    const FilePath &session_path = settings.GetLastSessionPath();

    const std::string file_name = ! session_path ? "" :
        Util::ReplaceString(session_path.GetFileName(), ".mvr", ".stl");

    return FilePath::Join(settings.GetExportDirectory(), file_name);
}

void SessionPanel::LoadSessionFromPath_(const FilePath &path) {
    ASSERT(path);

    // If changes were made in the current session (and it can therefore be
    // saved), ask the user what to do.
    if (GetContext().session_manager->CanSaveSession()) {
        const std::string msg = "There are unsaved changes."
            " Do you really want to load another session?";
        auto func = [&](const std::string &answer){
            if (answer == "Yes")
                ReallyLoadSessionFromPath_(path);
        };
        AskQuestion(msg, func);
    }
    else {
        ReallyLoadSessionFromPath_(path);
    }
}

void SessionPanel::ReallyLoadSessionFromPath_(const FilePath &path) {
    ASSERT(path);

    std::string error;
    if (GetContext().session_manager->LoadSession(path, error)) {
        Close("Done");
        SetLastSessionPath_(path);
    }
    else {
        DisplayMessage("Could not load session from '" +
                       path.ToString() + "':\n" + error, nullptr);
    }
}

void SessionPanel::SaveSessionToPath_(const FilePath &path) {
    ASSERT(path);
    Close("Done");
    if (GetContext().session_manager->SaveSession(path))
        SetLastSessionPath_(path);
}

void SessionPanel::ExportToPath_(const FilePath &path, FileFormat format) {
    ASSERT(path);
    Close("Done");

    const UnitConversion &conv = GetSettings().GetExportUnitsConversion();
    if (! GetContext().session_manager->Export(path, format, conv)) {
        DisplayMessage("Could not export selection to '" +
                       path.ToString() + "'", nullptr);
    }
}

void SessionPanel::SetLastSessionPath_(const FilePath &path) {
    // Copy the current settings.
    SettingsPtr new_settings = Settings::CreateDefault();
    new_settings->CopyFrom(GetSettings());

    new_settings->SetLastSessionPath(path);
    GetContext().settings_manager->SetSettings(*new_settings);
}
