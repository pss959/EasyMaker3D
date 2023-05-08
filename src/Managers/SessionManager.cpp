#include "Managers/SessionManager.h"

#include <fstream>

#include "Agents/ActionAgent.h"
#include "Commands/CommandList.h"
#include "IO/STLWriter.h"
#include "Items/UnitConversion.h"
#include "Managers/CommandManager.h"
#include "Managers/SelectionManager.h"
#include "Models/Model.h"
#include "Math/MeshUtils.h"
#include "Parser/Parser.h"
#include "Parser/Registry.h"
#include "Parser/Writer.h"
#include "Util/KLog.h"

SessionManager::SessionManager(const ActionAgentPtr &action_agent,
                               const CommandManagerPtr &command_manager,
                               const SelectionManagerPtr &selection_manager,
                               const FilePath &previous_path) :
    action_agent_(action_agent),
    command_manager_(command_manager),
    selection_manager_(selection_manager) {
    ASSERT(action_agent_);
    ASSERT(command_manager_);
    ASSERT(selection_manager_);

    original_session_state_ = Parser::Registry::CreateObject<SessionState>();
    ChangePreviousPath(previous_path);
}

void SessionManager::ChangePreviousPath(const FilePath &previous_path) {
    previous_session_name_ = GetSessionNameFromPath_(previous_path);
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
    if (command_list.DidCommandsChange())
        mods.Set(Modification::kCommands);
    if (! command_manager_->GetSessionState()->IsSameAs(
            *original_session_state_))
        mods.Set(Modification::kSessionState);
    return mods;
}

std::string SessionManager::GetSessionString() const {
    std::string session_string = ! current_session_name_.empty() ?
        current_session_name_ : "<Untitled Session>";

    const auto mods = GetModifications();
    if (mods.HasAny()) {
        session_string += " [";
        if (mods.Has(Modification::kScene))
            session_string += "*";
        if (mods.Has(Modification::kSessionState))
            session_string += "!";
        if (mods.Has(Modification::kCommands))
            session_string += "+";
        session_string += "]";
    }
    return session_string;
}

bool SessionManager::SaveSessionWithComments(
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

    current_session_name_ = GetSessionNameFromPath_(path);

    command_list.ClearChanges();
    SaveOriginalSessionState_();
    return true;
}

void SessionManager::NewSession() {
    ResetSession_();
}

bool SessionManager::SaveSession(const FilePath &path) {
    return SaveSessionWithComments(path, std::vector<std::string>());
}

bool SessionManager::LoadSession(const FilePath &path, std::string &error) {
    return LoadSessionSafe_(path, &error);
}

std::string SessionManager::GetModelNameForExport() const {
    const auto &sel = selection_manager_->GetSelection();
    return sel.HasAny() ? sel.GetPrimary().GetModel()->GetName() : "";
}

bool SessionManager::Export(const FilePath &path, FileFormat format,
                            const UnitConversion &conv) {
    // Collect Model meshes, transforming them into stage coordinates.
    const auto &sel = selection_manager_->GetSelection();
    std::vector<TriMesh> meshes;
    for (const auto &sel_path: sel.GetPaths()) {
        ASSERT(sel_path.GetModel());
        const Matrix4f osm = sel_path.GetCoordConv().GetObjectToRootMatrix();
        meshes.push_back(TransformMesh(sel_path.GetModel()->GetMesh(), osm));
    }

    return WriteSTLFile(meshes, path, format, conv.GetFactor());
}

const std::string & SessionManager::GetPreviousSessionName() const {
    return previous_session_name_;
}

const std::string & SessionManager::GetCurrentSessionName() const {
    return current_session_name_;
}

void SessionManager::ResetSession_() {
    previous_session_name_.clear();
    current_session_name_.clear();

    action_agent_->Reset();
    Model::ResetColors();
    SaveOriginalSessionState_();
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

        // Reselect now that the session is loaded so the correct Tool is
        // attached. Cannot use ReselectAll() here because Tools are not
        // currently attached.
        selection_manager_->ChangeSelection(selection_manager_->GetSelection());
    }
    catch (const Parser::Exception &ex) {
        KLOG('w', "Loading threw exception: " << ex.what());
        if (error)
            *error = ex.what();
        else
            throw;
        return false;
    }
    KLOG('w', "Loading was successful: "
         << command_manager_->GetCommandList()->GetCommandCount()
         << " command(s)");
    command_manager_->GetCommandList()->ClearChanges();
    current_session_name_ = GetSessionNameFromPath_(path);
    action_agent_->UpdateFromSessionState(*command_manager_->GetSessionState());
    SaveOriginalSessionState_();
    return true;
}

void SessionManager::SaveOriginalSessionState_() {
    original_session_state_->CopyFrom(*command_manager_->GetSessionState());
}

std::string SessionManager::GetSessionNameFromPath_(const FilePath &path) {
    return path.GetFileName(true);  // Removes extension.
}
