#include "Panels/SessionPanel.h"

#include "Panels/FilePanel.h"
#include "Settings.h"
#include "Util/Assert.h"
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
    auto &session_manager = GetContext().session_manager;

    const Util::FilePath &session_path = GetSettings().last_session_path;
    std::string continue_text;

    const bool is_continue_enabled =
        session_path.Exists() && ! session_path.IsDirectory();
    if (is_continue_enabled) {
        const bool is_current =
            session_manager->GetSessionPath() == session_path;
        continue_text = std::string("Continue ") +
            (is_current ? "current" : "previous") + " session " +
            session_path.GetFileName();
    }
    else {
        continue_text = "(No previous session)";
    }
    SetButtonText("Continue", continue_text);
    EnableButton("Continue", is_continue_enabled);
    EnableButton("Load", true);
    EnableButton("Save",
                 session_path.Exists() && session_manager->CanSaveSession());
    EnableButton("Export", session_manager->CanExport());

    // Move the focus to a button that is enabled.
    SetFocus(is_continue_enabled ? "Continue" : "New");
}

void SessionPanel::OpenHelp_() {
    Close(CloseReason::kReplaceAndRestore, "HelpPanel");
}

void SessionPanel::OpenSettings_() {
    Close(CloseReason::kReplaceAndRestore, "SettingsPanel");
}

void SessionPanel::ContinueSession_() {
    Close(CloseReason::kDone, "Done");

    // Do nothing if the session is already loaded.
    auto &session_manager = GetContext().session_manager;
    const auto &session_path = GetSettings().last_session_path;

    if (session_manager->GetSessionPath() != session_path) {
        ASSERT(session_path.Exists());
        session_manager->LoadSession(session_path);
    }
}

void SessionPanel::LoadSession_() {
    // XXXX TEMPORARY
    Close(CloseReason::kDone, "Done");
}

void SessionPanel::StartNewSession_() {
    // XXXX TEMPORARY
    Close(CloseReason::kDone, "Done");
}

void SessionPanel::SaveSession_(bool use_current_file) {
    if (use_current_file)
        SaveSessionToPath_(GetSettings().last_session_path);
    else
        ChooseFile_(FileTarget_::kSaveSession);
}

void SessionPanel::ExportSelection_() {
    // XXXX TEMPORARY
    Close(CloseReason::kDone, "Done");
}

void SessionPanel::SaveSessionToPath_(const Util::FilePath &path) {
    if (path) {
        std::cerr << "XXXX Saving to '" << path.ToString() << "'\n";
        auto &session_manager = GetContext().session_manager;
        Close(CloseReason::kDone, "Done");
        if (session_manager->SaveSession(path))
            SetLastSessionPath_(path);
    }
}

void SessionPanel::SetLastSessionPath_(const Util::FilePath &path) {
    auto settings_manager = GetContext().settings_manager;
    Settings settings = settings_manager->GetSettings();
    settings.last_session_path = path;
    settings_manager->SetSettings(settings);
}

void SessionPanel::InitReplacementPanel(Panel &new_panel) {
    // The new panel can be a HelpPanel, SettingsPanel, or FilePanel. Only the
    // FilePanel needs to be set up.
    if (new_panel.GetTypeName() != "FilePanel")
        return;

    FileTarget_ target = file_panel_target_;
    const Util::FilePath initial_path = GetInitialPath_(target);

    FilePanel &file_panel = static_cast<FilePanel &>(new_panel);
    file_panel.Reset();
    file_panel.SetTitle(GetFilePanelTitle_(target));
    file_panel.SetTargetType(target == FileTarget_::kLoadSession ?
                             FilePanel::TargetType::kExistingFile :
                             FilePanel::TargetType::kNewFile);
    file_panel.SetInitialPath(initial_path);
    if (target == FileTarget_::kExport) {
        /* XXXX Set up formats dropdown...
        List<FileFormat> formats =
                new List<FileFormat>(UT.EnumIterator<FileFormat>());
            // Remove the "Unknown" format.
            formats.Remove(FileFormat.Unknown);
            filePanel.SetFormats(formats);
        */
    }
    file_panel.SetExtension(target == FileTarget_::kExport ? "stl" : "mvr");
    if (target == FileTarget_::kExport)
        file_panel.SetHighlightPath(initial_path, "");
    else
        file_panel.SetHighlightPath(GetSettings().last_session_path,
                                    " [CURRENT SESSION]");
}

void SessionPanel::SetReplacementResult(Panel &prev_panel,
                                        const std::string &result) {
    // If the result was from a FilePanel and it was not canceled, process it.
    if (prev_panel.GetTypeName() == "FilePanel" && result == "Accept") {
        FilePanel &file_panel = static_cast<FilePanel &>(prev_panel);
        switch (file_panel_target_) {
          case FileTarget_::kLoadSession:
            // XXXX
            break;
          case FileTarget_::kSaveSession:
            SaveSessionToPath_(file_panel.GetPath());
            break;
          case FileTarget_::kExport:
            // XXXX
            break;
        }
    }
}

void SessionPanel::ChooseFile_(FileTarget_ target) {
    // Save the target so InitReplacementPanel() can operate.
    file_panel_target_ = target;
    Close(CloseReason::kReplaceAndRestore, "FilePanel");
}

std::string SessionPanel::GetFilePanelTitle_(FileTarget_ target) {
    switch (target) {
      case FileTarget_::kLoadSession:
        return "Select a session file (.mvr) to load";
      case FileTarget_::kSaveSession:
        return "Enter a session file (.mvr) to save to";
      case FileTarget_::kExport:
        return "Enter a file to export to";
      default:
        ASSERTM(false, "Bad FileTarget_ enum");
        return "";
    }
}

Util::FilePath SessionPanel::GetInitialPath_(FileTarget_ target) {
    const Settings &settings = GetSettings();

    if (target == FileTarget_::kExport) {
        // If the current session file is named "Foo.mvr", assume the STL
        // target file is "Foo.stl". Otherwise, leave the name blank.
        const Util::FilePath session_path = settings.last_session_path;
        const std::string file_name = ! session_path ? "" :
            Util::ReplaceString(session_path.GetFileName(), ".mvr", ".stl");
        return Util::FilePath::Join(GetSettings().export_directory, file_name);
    }
    else {
        return settings.session_directory;
    }
}

