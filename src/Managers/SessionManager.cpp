#include "Managers/SessionManager.h"

#include <fstream>

#include "Parser/Writer.h"

SessionManager::SessionManager(const CommandManagerPtr &command_manager,
                               const SelectionManagerPtr &selection_manager,
                               const ResetFunc &reset_func) :
    command_manager_(command_manager),
    selection_manager_(selection_manager),
    reset_func_(reset_func) {
    ASSERT(command_manager_);
    ASSERT(selection_manager_);
    ASSERT(reset_func_);
}

// XXXX Need real stuff here...

bool SessionManager::CanSaveSession() const {
    return GetModifications().HasAny();
}

void SessionManager::NewSession() {
    ResetSession_();
}

bool SessionManager::SaveSession(const Util::FilePath &path) {
    return SaveSessionWithComments_(path, std::vector<std::string>());
}

bool SessionManager::LoadSession(const Util::FilePath &path) {
    return LoadSessionSafe_(path, true);
}

const Util::FilePath & SessionManager::GetSessionPath() const {
    return session_path_;
}

bool SessionManager::CanExport() const {
    return selection_manager_->GetSelection().HasAny();
}

void SessionManager::ResetSession_() {
    reset_func_();
    // XXXX SaveOriginalSessionState();
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

bool SessionManager::LoadSessionSafe_(const Util::FilePath &path,
                                      bool catch_exceptions) {
#if XXXX
    try {
        ResetSession_();
        Parser::Parser parser;
        Parser::ObjectPtr root = parser.ParseFile(path);
        _commandManager.ClearChanges();
        UpdateSessionPath(path);
        _app.UpdateSessionState(_appInfo.sessionState);
        Report("Loaded session from <" + path + ">");
        SaveOriginalSessionState();
        return true;
    }
    catch (Exception ex) {
        if (catchExceptions) {
            _app.DisplaySessionError(
                $"Could not load session from '{path}'", ex.Message);
            return false;
        }
        else {
            throw;
        }
    }
#endif
    return false;
}
