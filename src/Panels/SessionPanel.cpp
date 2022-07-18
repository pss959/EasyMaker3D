#include "Panels/SessionPanel.h"

#include "Items/Settings.h"
#include "Managers/SessionManager.h"
#include "Managers/SettingsManager.h"
#include "Panels/FilePanel.h"
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

    // The Continue button is the most complicated. The possible versions of
    // this button are:
    //  (1) "(No previous session)"  [button disabled]
    //      This is shown when the application first starts if there is no
    //      session path saved in settings from a previous run.
    //  (2) "Continue previous session [name]"
    //      This is shown when the application first starts and there is a
    //      session path saved in settings from a previous run.
    //  (3) "Continue current session"
    //      This is shown when there is no session path saved in settings and
    //      the user has made changes in the current session.
    //  (4) "Continue current session [name]"
    //      This is shown when there is a session path saved in settings and
    //      the user has made changes in the current session.
    const FilePath &session_path    = GetSettings().GetLastSessionPath();
    const bool  changes_made        = session_manager.CanSaveSession();
    bool        continue_enabled    = true;
    std::string continue_text;

    if (! session_path) {
        if (changes_made) {
            continue_text = "Continue current session";
        }
        else {
            continue_text = "(No previous session)";
            continue_enabled = false;
        }
    }
    else {
        const std::string &session_name = session_manager.GetSessionName();
        ASSERT(! session_name.empty());
        continue_text = changes_made ?
            "Continue current session" : "Continue previous session";
        continue_text += " [" + session_name + "]";
    }

    const bool can_save = changes_made && session_path;
    SetButtonText("Continue", continue_text);
    EnableButton("Continue",  continue_enabled);
    EnableButton("Load",      true);
    EnableButton("Save",      can_save);
    EnableButton("Export",    session_manager.CanExport());

    // Move the focus to a button that is enabled unless there is already a
    // focused pane.
    if (! GetFocusedPane())
        SetFocus(continue_enabled ? "Continue" : "New");
}

void SessionPanel::Close(const std::string &result) {
    // Canceling starts a new session if this is the initial SessionPanel.
    if (result == "Cancel" &&
        ! GetContext().session_manager->SessionStarted()) {
        StartNewSession_();
    }
    else {
        Panel::Close(result);
    }
}

void SessionPanel::OpenHelp_() {
    auto &helper = *GetContext().panel_helper;
    helper.PushPanel(helper.GetPanel("HelpPanel"), nullptr);
}

void SessionPanel::OpenSettings_() {
    auto &helper = *GetContext().panel_helper;
    helper.PushPanel(helper.GetPanel("SettingsPanel"), nullptr);
}

void SessionPanel::ContinueSession_() {
    // The only time something needs to be done here is when the application
    // first starts and the user continued a previous session. In this case,
    // load the previous session file.
    auto &session_manager    = *GetContext().session_manager;
    const bool  changes_made = session_manager.CanSaveSession();
    const auto &session_path = GetSettings().GetLastSessionPath();
    if (session_path && ! changes_made) {
        std::string error;
        if (session_manager.LoadSession(session_path, error))
            Close("Done");
        else
            DisplayMessage("Could not load session from '" +
                           session_path.ToString() + "':\n" + error, nullptr);
    }
    else {
        Close("Done");
    }
}

void SessionPanel::LoadSession_() {
    // Access and set up the FilePanel.
    auto &helper = *GetContext().panel_helper;
    auto fp = helper.GetTypedPanel<FilePanel>("FilePanel");
    const auto &settings = GetSettings();
    fp->Reset();
    fp->SetTitle("Select a session file (.mvr) to load");
    fp->SetInitialPath(settings.GetSessionDirectory());
    fp->SetTargetType(FilePanel::TargetType::kExistingFile);
    fp->SetExtension(".mvr");
    fp->SetHighlightPath(settings.GetLastSessionPath(), " [CURRENT SESSION]");

    auto result_func = [&, fp](const std::string &result){
        if (result == "Accept")
            LoadSessionFromPath_(fp->GetPath());
    };
    helper.PushPanel(fp, result_func);
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
        // Access and set up the FilePanel.
        auto &helper = *GetContext().panel_helper;
        auto fp = helper.GetTypedPanel<FilePanel>("FilePanel");
        const auto &settings = GetSettings();
        fp->Reset();
        fp->SetTitle("Enter a session file (.mvr) to save to");
        fp->SetInitialPath(settings.GetSessionDirectory());
        fp->SetTargetType(FilePanel::TargetType::kNewFile);
        fp->SetExtension(".mvr");
        fp->SetHighlightPath(settings.GetLastSessionPath(),
                            " [CURRENT SESSION]");

        auto result_func = [&, fp](const std::string &result){
            if (result == "Accept")
                SaveSessionToPath_(fp->GetPath());
        };
        helper.PushPanel(fp, result_func);
    }
}

void SessionPanel::ExportSelection_() {
    // Access and set up the FilePanel.
    auto &helper = *GetContext().panel_helper;
    auto fp = helper.GetTypedPanel<FilePanel>("FilePanel");
    fp->Reset();
    fp->SetTitle("Enter a file to export to");
    fp->SetInitialPath(GetInitialExportPath_());
    fp->SetTargetType(FilePanel::TargetType::kNewFile);
    fp->SetExtension(".stl");
    std::vector<FileFormat> formats;
    for (auto &format: Util::EnumValues<FileFormat>()) {
        if (format != FileFormat::kUnknown)
            formats.push_back(format);
    }
    fp->SetFileFormats(formats);

    auto result_func = [&, fp](const std::string &result){
        if (result == "Accept") {
            const FileFormat format = fp->GetFileFormat();
            ASSERT(format != FileFormat::kUnknown);
            ExportToPath_(fp->GetPath(), format);
        }
    };
    helper.PushPanel(fp, result_func);
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
