#include "Panels/SessionPanel.h"

#include "Panels/SessionPanel.h"
#include "Util/FilePath.h"

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

void SessionPanel::ProcessButton(const std::string &name) {
    std::cerr << "XXXX Got button '" << name << "'\n";

    // XXXX TEMPORARY
    Close("Done");
}
