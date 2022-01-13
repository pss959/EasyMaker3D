#include "Panels/SessionPanel.h"

#include "Panels/DialogPanel.h"
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

    const FilePath &session_path = GetSettings().last_session_path;
    std::string continue_text;

    const bool have_session = session_path.Exists();
    const bool can_continue = have_session && ! session_path.IsDirectory();
    if (can_continue) {
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
    EnableButton("Continue", can_continue);
    EnableButton("Load",     true);
    EnableButton("Save",     have_session && session_manager->CanSaveSession());
    EnableButton("Export",   session_manager->CanExport());

    // Move the focus to a button that is enabled.
    SetFocus(can_continue ? "Continue" : "New");
}

void SessionPanel::OpenHelp_() {
    GetContext().panel_helper->Replace("HelpPanel", nullptr, nullptr);
}

void SessionPanel::OpenSettings_() {
    GetContext().panel_helper->Replace("SettingsPanel", nullptr, nullptr);
}

void SessionPanel::ContinueSession_() {
    Close("Done");

    // Do nothing if the session is already loaded.
    auto &session_manager = GetContext().session_manager;
    const auto &session_path = GetSettings().last_session_path;

    if (session_manager->GetSessionPath() != session_path) {
        ASSERT(session_path.Exists());
        session_manager->LoadSession(session_path);
    }
}

void SessionPanel::LoadSession_() {
    auto init = [&](Panel &p){
        ASSERT(p.GetTypeName() == "FilePanel");
        FilePanel &fp = static_cast<FilePanel &>(p);
        const auto &settings = GetSettings();
        fp.Reset();
        fp.SetTitle("Select a session file (.mvr) to load");
        fp.SetInitialPath(settings.session_directory);
        fp.SetTargetType(FilePanel::TargetType::kExistingFile);
        fp.SetExtension(".mvr");
        fp.SetHighlightPath(settings.last_session_path, " [CURRENT SESSION]");
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
    auto &session_manager = GetContext().session_manager;

    auto do_start_new = [&](){
        Close("Done");
        session_manager->NewSession();
    };

    // If changes were made in the current session (and it can therefore be
    // saved), ask the user what to do.
    if (session_manager->CanSaveSession()) {
        const std::string msg = "There are unsaved changes."
            " Do you really want to start a new session?";
        auto func = [&](const std::string &answer){
            if (answer == "Yes")
                do_start_new();
        };
        AskQuestion(msg, func);
    }
    else {
        do_start_new();
    }
}

void SessionPanel::SaveSession_(bool use_current_file) {
    if (use_current_file) {
        SaveSessionToPath_(GetSettings().last_session_path);
    }
    else {
        auto init = [&](Panel &p){
            ASSERT(p.GetTypeName() == "FilePanel");
            FilePanel &fp = static_cast<FilePanel &>(p);
            const auto &settings = GetSettings();
            fp.Reset();
            fp.SetTitle("Enter a session file (.mvr) to save to");
            fp.SetInitialPath(settings.session_directory);
            fp.SetTargetType(FilePanel::TargetType::kNewFile);
            fp.SetExtension(".mvr");
            fp.SetHighlightPath(settings.last_session_path,
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
    auto init = [&](Panel &p){
        ASSERT(p.GetTypeName() == "FilePanel");
        FilePanel &fp = static_cast<FilePanel &>(p);
        fp.Reset();
        fp.SetTitle("Enter a file to export to");
        fp.SetInitialPath(GetInitialExportPath_());
        fp.SetTargetType(FilePanel::TargetType::kNewFile);
        fp.SetExtension(".stl");
        /* XXXX Set up formats dropdown...
        List<FileFormat> formats =
                new List<FileFormat>(UT.EnumIterator<FileFormat>());
            // Remove the "Unknown" format.
            formats.Remove(FileFormat.Unknown);
            filePanel.SetFormats(formats);
        */
    };
    auto result = [&](Panel &p, const std::string &res){
        ASSERT(p.GetTypeName() == "FilePanel");
        FilePanel &fp = static_cast<FilePanel &>(p);
        if (res == "Accept")
            ExportToPath_(fp.GetPath());
    };
    GetContext().panel_helper->Replace("FilePanel", init, result);
}

FilePath SessionPanel::GetInitialExportPath_() {
    // If the current session file is named "Foo.mvr", assume the STL target
    // file is "Foo.stl". Otherwise, leave the name blank.
    const auto &settings = GetSettings();
    const FilePath &session_path = settings.last_session_path;

    const std::string file_name = ! session_path ? "" :
        Util::ReplaceString(session_path.GetFileName(), ".mvr", ".stl");

    return FilePath::Join(settings.export_directory, file_name);
}

void SessionPanel::LoadSessionFromPath_(const FilePath &path) {
    ASSERT(path);
    auto &session_manager = GetContext().session_manager;

    auto do_load = [&](){
        Close("Done");
        if (session_manager->LoadSession(path))
            SetLastSessionPath_(path);
    };

    // If changes were made in the current session (and it can therefore be
    // saved), ask the user what to do.
    if (session_manager->CanSaveSession()) {
        const std::string msg = "There are unsaved changes."
            " Do you really want to load another session?";
        auto func = [&](const std::string &answer){
            if (answer == "Yes")
                do_load();
        };
        AskQuestion(msg, func);
    }
    else {
        do_load();
    }
}

void SessionPanel::SaveSessionToPath_(const FilePath &path) {
    ASSERT(path);
    Close("Done");
    std::cerr << "XXXX Saving to '" << path.ToString() << "'\n";
    if (GetContext().session_manager->SaveSession(path))
        SetLastSessionPath_(path);
}

void SessionPanel::ExportToPath_(const FilePath &path) {
    // XXXX Do something...
}

void SessionPanel::SetLastSessionPath_(const FilePath &path) {
    auto settings_manager = GetContext().settings_manager;
    Settings settings = settings_manager->GetSettings();
    settings.last_session_path = path;
    settings_manager->SetSettings(settings);
}
