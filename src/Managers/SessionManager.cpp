#include "Managers/SessionManager.h"

#include <fstream>

#include "Parser/Parser.h"
#include "Parser/Registry.h"
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

    original_session_state_ = Parser::Registry::CreateObject<SessionState>();
}

bool SessionManager::CanSaveSession() const {
    return GetModifications().HasAny();
}

Util::Flags<SessionManager::Modification>
SessionManager::GetModifications() const {
    Util::Flags<Modification> mods;
    const auto &command_list = command_manager_->GetCommandList();
    if (command_list.AreAnyChanges())
        mods.Set(Modification::kScene);
    if (command_list.WasAnyCommandAdded())
        mods.Set(Modification::kCommands);
    if (! command_manager_->GetSessionState().IsSameAs(*original_session_state_))
        mods.Set(Modification::kSessionState);
    return mods;
}

void SessionManager::NewSession() {
    ResetSession_();
}

bool SessionManager::SaveSession(const FilePath &path) {
    return SaveSessionWithComments_(path, std::vector<std::string>());
}

bool SessionManager::LoadSession(const FilePath &path) {
    return LoadSessionSafe_(path, true);
}

const FilePath & SessionManager::GetSessionPath() const {
    return session_path_;
}

bool SessionManager::CanExport() const {
    return selection_manager_->GetSelection().HasAny();
}

void SessionManager::ResetSession_() {
    reset_func_();
    SaveOriginalSessionState_();
}

bool SessionManager::SaveSessionWithComments_(
    const FilePath &path, const std::vector<std::string> &comments) {

    std::ofstream out(path.ToNativeString());
    if (out.fail())
        return false;

    Parser::Writer writer(out);

    for (const auto &comment: comments)
        writer.WriteComment(comment);

    auto &command_list = command_manager_->GetCommandList();
    writer.WriteObject(command_list);

    command_list.ClearChanges();
    SetSessionPath_(path);
    SaveOriginalSessionState_();
    return true;
}

bool SessionManager::LoadSessionSafe_(const FilePath &path,
                                      bool catch_exceptions) {
    ResetSession_();
    try {
        Parser::Parser parser;
        Parser::ObjectPtr root = parser.ParseFile(path);
        // XXXX Verify that root is a CommandList.
        // XXXX Tell the command_manager_ to set and execute the CommandList.
    }
    catch (const Parser::Exception &ex) {
        if (catch_exceptions) {
            std::cerr << "XXXX PARSE ERROR: " << ex.what() << "\n";
            //_app.DisplaySessionError(
            //$"Could not load session from '{path}'", ex.Message);
            return false;
        }
        else {
            throw;
        }
    }
    command_manager_->GetCommandList().ClearChanges();
    SetSessionPath_(path);
    // XXXX Update ActionManager toggles from current SessionState.
    SaveOriginalSessionState_();
    return true;
}

void SessionManager::SetSessionPath_(const FilePath &path) {
    session_path_ = path;
    // XXXX tree_panel->SetSessionPath(path);
}

void SessionManager::SaveOriginalSessionState_() {
    original_session_state_->CopyFrom(command_manager_->GetSessionState());
}
