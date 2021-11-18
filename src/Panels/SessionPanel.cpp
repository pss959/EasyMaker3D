#include "Panels/SessionPanel.h"

#include "Util/FilePath.h"

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
                 ! session_path.empty() && session_manager->CanSaveSession());
    EnableButton("Export", session_manager->CanExport());

    // Move the focus to a button that is enabled.
    SetFocus(is_continue_enabled ? "Continue" : "New");
}

void SessionPanel::OpenHelp_() {
    Close(CloseReason::kReplaceAndRestore, "HelpPanel");
}

void SessionPanel::OpenSettings_() {
    // XXXX TEMPORARY
    Close(CloseReason::kDone, "Done");
}

void SessionPanel::ContinueSession_() {
    // XXXX TEMPORARY
    Close(CloseReason::kDone, "Done");
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
    // XXXX TEMPORARY
    Close(CloseReason::kDone, "Done");
}

void SessionPanel::ExportSelection_() {
    // XXXX TEMPORARY
    Close(CloseReason::kDone, "Done");
}
