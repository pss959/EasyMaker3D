#include "Managers/SessionManager.h"

SessionManager::SessionManager(const CommandManagerPtr &command_manager,
                               const SelectionManagerPtr &selection_manager) :
    command_manager_(command_manager),
    selection_manager_(selection_manager) {
}

// XXXX Need real stuff here...

bool SessionManager::CanSaveSession() const {
    return GetModifications().HasAny();
}

const Util::FilePath & SessionManager::GetSessionPath() const {
    return session_path_;
}

bool SessionManager::CanExport() const {
    return selection_manager_->GetSelection().HasAny();
}
