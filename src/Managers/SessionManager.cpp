#include "Managers/SessionManager.h"

#include <fstream>

#include "Parser/Writer.h"

SessionManager::SessionManager(const CommandManagerPtr &command_manager,
                               const SelectionManagerPtr &selection_manager) :
    command_manager_(command_manager),
    selection_manager_(selection_manager) {
}

// XXXX Need real stuff here...

bool SessionManager::CanSaveSession() const {
    return GetModifications().HasAny();
}

bool SessionManager::SaveSession(const Util::FilePath &path) {
    return SaveSessionWithComments_(path, std::vector<std::string>());
}

const Util::FilePath & SessionManager::GetSessionPath() const {
    return session_path_;
}

bool SessionManager::CanExport() const {
    return selection_manager_->GetSelection().HasAny();
}

bool SessionManager::SaveSessionWithComments_(
    const Util::FilePath &path, const std::vector<std::string> &comments) {

    std::ofstream out(path.ToNativeString());
    if (out.fail())
        return false;

    Parser::Writer writer(out);

    for (const auto &comment: comments)
        writer.WriteComment(comment);

    auto &command_list = command_manager_->GetCommandList();
    writer.WriteObject(command_list);

    command_list.ClearChanges();
    session_path_ = path;
    // XXXX SaveOriginalSessionState_();
    return true;
}
