#include "Managers/SessionManager.h"

#include <fstream>

#include "Commands/CommandList.h"
#include "IO/STLWriter.h"
#include "Items/UnitConversion.h"
#include "Managers/ActionManager.h"
#include "Managers/CommandManager.h"
#include "Managers/SelectionManager.h"
#include "Parser/Parser.h"
#include "Parser/Registry.h"
#include "Parser/Writer.h"
#include "Util/KLog.h"

SessionManager::SessionManager(const ActionManagerPtr &action_manager,
                               const CommandManagerPtr &command_manager,
                               const SelectionManagerPtr &selection_manager) :
    action_manager_(action_manager),
    command_manager_(command_manager),
    selection_manager_(selection_manager) {
    ASSERT(action_manager_);
    ASSERT(command_manager_);
    ASSERT(selection_manager_);

    original_session_state_ = Parser::Registry::CreateObject<SessionState>();
}

bool SessionManager::CanSaveSession() const {
    return GetModifications().HasAny();
}

Util::Flags<SessionManager::Modification>
SessionManager::GetModifications() const {
    Util::Flags<Modification> mods;
    const auto &command_list = *command_manager_->GetCommandList();
    if (command_list.AreAnyChanges())
        mods.Set(Modification::kScene);
    if (command_list.WasAnyCommandAdded())
        mods.Set(Modification::kCommands);
    if (! command_manager_->GetSessionState()->IsSameAs(
            *original_session_state_))
        mods.Set(Modification::kSessionState);
    return mods;
}

bool SessionManager::SessionStarted() const {
    return GetSessionPath() || GetModifications().HasAny();
}

void SessionManager::NewSession() {
    ResetSession_();
}

bool SessionManager::SaveSession(const FilePath &path) {
    return SaveSessionWithComments_(path, std::vector<std::string>());
}

bool SessionManager::LoadSession(const FilePath &path, std::string &error) {
    return LoadSessionSafe_(path, &error);
}

const FilePath & SessionManager::GetSessionPath() const {
    return session_path_;
}

bool SessionManager::CanExport() const {
    return selection_manager_->GetSelection().HasAny();
}

bool SessionManager::Export(const FilePath &path, FileFormat format,
                            const UnitConversion &conv) {
    return WriteSTLFile(selection_manager_->GetSelection(), path, format,
                        conv.GetFactor());
}

std::string SessionManager::GetSessionString() const {
    std::string s =
        session_name_.empty() ? "<Untitled Session>" : session_name_;
    const auto mods = GetModifications();
    if (mods.HasAny()) {
        s += " [";
        if (mods.Has(Modification::kScene))
            s += "*";
        if (mods.Has(Modification::kSessionState))
            s += "!";
        if (mods.Has(Modification::kCommands))
            s += "+";
        s += "]";
    }
    return s;
}

void SessionManager::ResetSession_() {
    session_path_.Clear();
    session_name_.clear();
    action_manager_->Reset();
    SaveOriginalSessionState_();
}

bool SessionManager::SaveSessionWithComments_(
    const FilePath &path, const std::vector<std::string> &comments) {

    KLOG('w', "Saving session to '" << path.ToNativeString() << "'");
    std::ofstream out(path.ToNativeString());
    if (out.fail())
        return false;

    Parser::Writer writer(out);

    for (const auto &comment: comments)
        writer.WriteComment(comment);

    auto &command_list = *command_manager_->GetCommandList();
    writer.WriteObject(command_list);

    command_list.ClearChanges();
    SetSessionPath_(path);
    SaveOriginalSessionState_();
    return true;
}

bool SessionManager::LoadSessionSafe_(const FilePath &path,
                                      std::string *error) {
    ResetSession_();
    KLOG('w', "Loading session from '" << path.ToString() << "'");
    try {
        Parser::Parser parser;
        Parser::ObjectPtr root = parser.ParseFile(path);
        ASSERT(root);
        if (root->GetTypeName() != "CommandList")
            throw Parser::Exception(path, "Expected a CommandList; got " +
                                    root->GetTypeName());
        command_manager_->ProcessCommandList(
            Util::CastToDerived<CommandList>(root));
    }
    catch (const Parser::Exception &ex) {
        if (error)
            *error = ex.what();
        else
            throw;
        return false;
    }
    command_manager_->GetCommandList()->ClearChanges();
    SetSessionPath_(path);
    action_manager_->UpdateFromSessionState(
        *command_manager_->GetSessionState());
    SaveOriginalSessionState_();
    return true;
}

void SessionManager::SetSessionPath_(const FilePath &path) {
    session_path_ = path;
    session_name_ = path.GetFileName(true);  // Removes extension.
}

void SessionManager::SaveOriginalSessionState_() {
    original_session_state_->CopyFrom(*command_manager_->GetSessionState());
}
