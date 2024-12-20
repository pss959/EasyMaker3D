//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Panels/SessionPanel.h"

#include "Agents/BoardAgent.h"
#include "Agents/SessionAgent.h"
#include "Agents/SettingsAgent.h"
#include "Items/Settings.h"
#include "Panels/FilePanel.h"
#include "Util/Assert.h"
#include "Util/String.h"
#include "Util/Tuning.h"

void SessionPanel::InitInterface() {
    extension_ = TK::kSessionFileExtension;

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
    auto &session_agent = *GetContext().session_agent;

    // The Continue button is the most complicated. The possible versions of
    // this button are:
    //  (1) "(No previous session)"  (button disabled)
    //      This is shown when the application first starts if there is no
    //      previous session name.
    //  (2) "Continue previous session [name]"
    //      This is shown when the application first starts and there is a
    //      previous session name.
    //  (3) "Continue current session"
    //      This is shown when the current session is not yet named but has
    //      changes.
    //  (4) "Continue current session [name]"
    //      This is shown when the current session has a name.
    bool        have_name = false;
    Str continue_text;

    const auto &prev_name  = session_agent.GetPreviousSessionName();
    const auto &cur_name   = session_agent.GetCurrentSessionName();
    const bool has_changes = session_agent.CanSaveSession();

    // Have a current or previous session name.
    auto add_name = [](const Str &msg, const Str &name){
        return msg  + " [" + name + "]";
    };
    if (! cur_name.empty()) {
        continue_text = add_name("Continue current session", cur_name);
        have_name = true;
    }
    else if (! prev_name.empty()) {
        continue_text = add_name("Continue previous session", prev_name);
        have_name = true;
    }
    else if (has_changes) {
        continue_text = "Continue current session";
    }
    else {
        continue_text = "(No previous session)";
    }
    SetButtonText("Continue", continue_text);
    EnableButton("Continue",  have_name || has_changes);

    EnableButton("Load",   true);
    EnableButton("Save",   have_name && has_changes);
    EnableButton("Export", ! session_agent.GetModelNameForExport().empty());

    // Move the focus to a button that is enabled unless there is already a
    // focused pane.
    if (! GetFocusedPane())
        SetFocus(have_name ? "Continue" : "New");
}

void SessionPanel::Close(const Str &result) {
    // Rules for Cancel:
    //  If a previous session exists but no changes have been made and the
    //  current session has no name, start a new session. Otherwise, just
    //  continue the current session.
    const auto &sa = *GetContext().session_agent;
    if (result == "Cancel" &&
        ! sa.GetPreviousSessionName().empty() && ! sa.CanSaveSession())
        StartNewSession_();
    else
        Panel::Close(result);
}

void SessionPanel::OpenHelp_() {
    auto &ba = *GetContext().board_agent;
    ba.PushPanel(ba.GetPanel("HelpPanel"), nullptr);
}

void SessionPanel::OpenSettings_() {
    auto &ba = *GetContext().board_agent;
    ba.PushPanel(ba.GetPanel("SettingsPanel"), nullptr);
}

void SessionPanel::ContinueSession_() {
    // The only time something needs to be done here is when the application
    // first starts (i.e., there is no current session name) and the user
    // continued a previous session. In this case, load the previous session
    // file.
    auto &session_agent      = *GetContext().session_agent;
    const bool changes_made  = session_agent.CanSaveSession();
    const auto &session_path = GetSettings().GetLastSessionPath();

    if (session_agent.GetCurrentSessionName().empty() && session_path &&
        ! changes_made) {
        Str error;
        if (! session_agent.LoadSession(session_path, error)) {
            DisplayMessage("Could not load session from '" +
                           session_path.ToString() + "':\n" + error);
        }
        else {
            Close("Done");
        }
    }
}

void SessionPanel::LoadSession_() {
    // If a session was already loaded, use the same directory. Otherwise, use
    // the session directory from the settings.
    const auto &settings = GetSettings();
    FilePath initial_path = settings.GetLastSessionPath();
    if (! initial_path)
        initial_path = settings.GetSessionDirectory();

    // Access and set up the FilePanel.
    auto fp = GetTypedPanel<FilePanel>("FilePanel");
    fp->Reset();
    fp->SetTitle("Select a session file (" + extension_ + ") to load");
    fp->SetInitialPath(initial_path);
    fp->SetTargetType(FilePanel::TargetType::kExistingFile);
    fp->SetExtension(extension_);
    fp->SetHighlightPath(settings.GetLastSessionPath(), " [CURRENT SESSION]");

    auto result_func = [&, fp](const Str &result){
        if (result == "Accept")
            LoadSessionFromPath_(fp->GetPath());
    };
    GetContext().board_agent->PushPanel(fp, result_func);
}

void SessionPanel::StartNewSession_() {
    // If changes were made in the current session (and it can therefore be
    // saved), ask the user what to do.
    if (GetContext().session_agent->CanSaveSession()) {
        const Str msg = "There are unsaved changes."
            " Do you really want to start a new session?";
        auto func = [&](const Str &answer){
            if (answer == "Yes") {
                Close("Done");
                GetContext().session_agent->NewSession();
            }
        };
        AskQuestion(msg, func, true);
    }
    else {
        Close("Done");
        GetContext().session_agent->NewSession();
        SetLastSessionPath_(FilePath());
    }
}

void SessionPanel::SaveSession_(bool use_current_file) {
    if (use_current_file) {
        SaveSessionToPath_(GetSettings().GetLastSessionPath());
    }
    else {
        // Access and set up the FilePanel.
        auto fp = GetTypedPanel<FilePanel>("FilePanel");
        const auto &settings = GetSettings();
        fp->Reset();
        fp->SetTitle("Enter a session file (" + extension_ + ") to save to");
        fp->SetInitialPath(settings.GetSessionDirectory());
        fp->SetTargetType(FilePanel::TargetType::kNewFile);
        fp->SetExtension(extension_);
        fp->SetHighlightPath(settings.GetLastSessionPath(),
                            " [CURRENT SESSION]");

        auto result_func = [&, fp](const Str &result){
            if (result == "Accept")
                SaveSessionToPath_(fp->GetPath());
        };
        GetContext().board_agent->PushPanel(fp, result_func);
    }
}

void SessionPanel::ExportSelection_() {
    // Access and set up the FilePanel.
    auto fp = GetTypedPanel<FilePanel>("FilePanel");
    fp->Reset();
    fp->SetTitle("Enter a file to export to");
    fp->SetInitialPath(GetInitialExportPath_());
    fp->SetExtension(".stl");  // Matches default format.
    fp->SetTargetType(FilePanel::TargetType::kNewFile);
    std::vector<FileFormat> formats;
    for (auto &format: Util::EnumValues<FileFormat>()) {
        if (format != FileFormat::kUnknown)
            formats.push_back(format);
    }
    fp->SetFileFormats(formats);

    auto result_func = [&, fp](const Str &result){
        if (result == "Accept") {
            const FileFormat format = fp->GetFileFormat();
            ASSERT(format != FileFormat::kUnknown);
            ExportToPath_(fp->GetPath(), format);
        }
    };
    GetContext().board_agent->PushPanel(fp, result_func);
}

FilePath SessionPanel::GetInitialExportPath_() {
    // Get the name of the selected Model and use it for the path.
    const Str model_name = GetContext().session_agent->GetModelNameForExport();
    ASSERT(! model_name.empty());

    return FilePath::Join(GetSettings().GetExportDirectory(),
                          model_name + ".stl");
}

void SessionPanel::LoadSessionFromPath_(const FilePath &path) {
    ASSERT(path);

    // If changes were made in the current session (and it can therefore be
    // saved), ask the user what to do.
    if (GetContext().session_agent->CanSaveSession()) {
        const Str msg = "There are unsaved changes."
            " Do you really want to load another session?";
        auto func = [&](const Str &answer){
            if (answer == "Yes") {
                ReallyLoadSessionFromPath_(path);
            }
        };
        AskQuestion(msg, func, true);
    }
    else {
        ReallyLoadSessionFromPath_(path);
    }
}

void SessionPanel::ReallyLoadSessionFromPath_(const FilePath &path) {
    ASSERT(path);

    Str error;
    if (GetContext().session_agent->LoadSession(path, error)) {
        Close("Done");
        SetLastSessionPath_(path);
    }
    else {
        DisplayMessage("Could not load session from '" +
                       path.ToString() + "':\n" + error);
    }
}

void SessionPanel::SaveSessionToPath_(const FilePath &path) {
    ASSERT(path);
    Close("Done");
    if (GetContext().session_agent->SaveSession(path))
        SetLastSessionPath_(path);
}

void SessionPanel::ExportToPath_(const FilePath &path, FileFormat format) {
    ASSERT(path);

    const UnitConversion &conv = GetSettings().GetExportUnitsConversion();
    if (GetContext().session_agent->Export(path, format, conv)) {
        Close("Done");
    }
    else {
        DisplayMessage("Could not export selection to '" +
                       path.ToString() + "'");
    }
}

void SessionPanel::SetLastSessionPath_(const FilePath &path) {
    // Copy the current settings.
    SettingsPtr new_settings = Settings::CreateCopy(GetSettings());
    new_settings->SetLastSessionPath(path);
    GetContext().settings_agent->SetSettings(*new_settings);
}
