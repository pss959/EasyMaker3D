#include "Managers/ToolManager.h"

#include <vector>

#include "Managers/InstanceManager.h"
#include "Managers/TargetManager.h"
#include "Models/Model.h"
#include "Tools/PassiveTool.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/KLog.h"

ToolManager::ToolManager(TargetManager &target_manager) :
    passive_tool_manager_(new InstanceManager) {
    // Attach a callback to the TargetManager to turn off active tools while
    // the target is being dragged so the tool geometry does not interfere with
    // target placement.
    target_manager.GetTargetActivation().AddObserver(
        this, [&](bool is_activation){ TargetActivated_(is_activation); });
}

void ToolManager::SetParentNode(const SG::NodePtr &parent_node) {
    parent_node_ = parent_node;
}

void ToolManager::AddTools(const std::vector<ToolPtr> &tools) {
    // This is used as the completion function for all specialized Tools. This
    // just toggles back to an appropriate general Tool.
    auto completion_func = [&](){
        ToggleSpecializedTool(GetCurrentTool()->GetSelection());
    };

    for (auto &tool: tools) {
        const std::string &type_name = tool->GetTypeName();

        // Special case for the PassiveTool.
        if (PassiveToolPtr pt = Util::CastToDerived<PassiveTool>(tool)) {
            KLOG('T', "Adding PassiveTool");
            passive_tool_manager_->AddOriginal<PassiveTool>(pt);
        }
        else {
            ASSERTM(! Util::MapContains(tool_name_map_, type_name), type_name);
            tool_name_map_[type_name] = tool;
            if (tool->IsSpecialized()) {
                KLOG('T', "Adding specialized " << type_name);
                tool->SetSpecializedCompletionFunc(completion_func);
                specialized_tools_.push_back(tool);
            }
            else {
                KLOG('T', "Adding general " << type_name);
                general_tools_.push_back(tool);
            }
        }
    }

    // Make sure there is a PassiveTool installed.
    ASSERT(passive_tool_manager_->HasOriginal<PassiveTool>());
}

void ToolManager::SetDefaultGeneralTool(const std::string &name) {
    ASSERT(! current_general_tool_);
    auto tool = tool_name_map_.at(name);
    ASSERT(tool);
    ASSERT(! tool->IsSpecialized());
    default_general_tool_ = tool;
    current_general_tool_ = default_general_tool_;
}

void ToolManager::ClearTools() {
    Reset();
    tool_name_map_.clear();
    general_tools_.clear();
    specialized_tools_.clear();
    current_general_tool_.reset();
    current_specialized_tool_.reset();
    default_general_tool_.reset();
    is_using_specialized_tool_ = false;
}

void ToolManager::Reset() {
    ResetSession();
    passive_tool_manager_->Reset();
}

void ToolManager::ResetSession() {
    // Detach all attached tools
    for (auto &tool: Util::GetValues(active_tool_map_))
        tool->DetachFromSelection();
    active_tool_map_.clear();
    current_general_tool_ = default_general_tool_;
    current_specialized_tool_.reset();
    is_using_specialized_tool_ = false;
}

bool ToolManager::CanUseGeneralTool(const std::string &name,
                                    const Selection &sel) {
    // There has to be at least one Model selected and the correct tool has to
    // support attaching to the selection.
    return sel.HasAny() && GetGeneralTool_(name)->CanBeUsedFor(sel);
}

bool ToolManager::CanUseSpecializedTool(const Selection &sel) {
    return GetSpecializedToolForSelection(sel).get();
}

ToolPtr ToolManager::GetSpecializedToolForSelection(
    const Selection &sel) {
    // Find a SpecializedTool that can attach to the selection.
    auto it = std::find_if(specialized_tools_.begin(),
                           specialized_tools_.end(),
                           [sel](const ToolPtr &tool){
                           return tool->CanBeUsedFor(sel); });
    return it != specialized_tools_.end() ? *it : ToolPtr();
}

void ToolManager::UseGeneralTool(const std::string &name,
                                 const Selection &sel) {
    UseTool_(GetGeneralTool_(name), sel);
}

void ToolManager::UseSpecializedTool(const Selection &sel) {
    if (! is_using_specialized_tool_ && CanUseSpecializedTool(sel))
        ToggleSpecializedTool(sel);
}

void ToolManager::ToggleSpecializedTool(const Selection &sel) {
    ToolPtr new_tool;

    // Switch to using a specialized tool if not using one now.
    if (! is_using_specialized_tool_) {
        new_tool = GetSpecializedToolForSelection(sel);
        if (new_tool)
            is_using_specialized_tool_ = true;
    }

    // Otherwise, use a general tool.
    if (! new_tool) {
        is_using_specialized_tool_ = false;
        ASSERT(current_general_tool_);
        new_tool = current_general_tool_;
    }

    UseTool_(new_tool, sel);
}

ToolPtr ToolManager::GetCurrentTool() const {
    if (is_using_specialized_tool_)
        return current_specialized_tool_;
    else
        return current_general_tool_;
}

void ToolManager::AttachToSelection(const Selection &sel) {
    // Nothing to do if the selection is empty.
    if (! sel.HasAny())
        return;

    ToolPtr tool;

    // Get the specialized tool if using one and there is one for the
    // selection.
    if (is_using_specialized_tool_) {
        current_specialized_tool_ = GetSpecializedToolForSelection(sel);
        tool = current_specialized_tool_;
    }

    // Not using a specialized tool.
    if (! tool) {
        ASSERT(current_general_tool_);
        is_using_specialized_tool_ = false;
        if (! current_general_tool_->CanBeUsedFor(sel))
            current_general_tool_ = GetPreviousOrNextGeneralTool_(sel, true);
        tool = current_general_tool_;
    }

    // Attach the proper tool for the primary selection.
    ASSERT(tool);
    UseTool_(tool, sel);

    // Attach a PassiveTool to all secondary selections.
    const size_t count = sel.GetCount();
    for (size_t i = 1; i < count; ++i) {
        auto pt = passive_tool_manager_->Acquire<PassiveTool>();
        AttachToolToModel_(pt, sel, i);
    }
}

void ToolManager::DetachTools(const Selection &sel) {
    for (auto &path: sel.GetPaths())
        DetachToolFromModel_(*path.GetModel());
}

void ToolManager::DetachAllTools() {
    // Save pointers to all attached Models from the map; the detaching code
    // removes entries from the map.
    const auto models = Util::GetKeys(active_tool_map_);

    for (auto &model: models)
        DetachToolFromModel_(*model);
}

void ToolManager::ReattachTools() {
    ASSERT(! is_tool_dragging_);

    // Set this flag temporarily so that any changes to a Model caused by
    // reattaching to it do not trigger yet another reattachment.
    is_tool_dragging_ = true;
    for (auto &tool: Util::GetValues(active_tool_map_))
        tool->ReattachToSelection();
    is_tool_dragging_ = false;
}

ToolPtr ToolManager::GetAttachedTool(const ModelPtr &model) const {
    return Util::MapContains(active_tool_map_, model.get()) ?
        active_tool_map_.at(model.get()) : ToolPtr();
}

const SG::Node * ToolManager::GetGrippableNode() const {
    auto tool = GetCurrentTool();
    return tool ? tool->GetGrippableNode() : nullptr;
}

GripGuideType ToolManager::GetGripGuideType() const {
    // This should not be called unless there is a current Tool.
    ASSERT(GetCurrentTool());
    return GetCurrentTool()->GetGripGuideType();
}

void ToolManager::UpdateGripInfo(GripInfo &info) {
    // This should not be called unless there is a current Tool.
    ASSERT(GetCurrentTool());

    // If the current Tool is not attached, there is nothing to update.
    auto tool = GetCurrentTool();
    if (tool->GetModelAttachedTo())
        tool->UpdateGripInfo(info);
}

ToolPtr ToolManager::GetGeneralTool_(const std::string &name) const {
    ASSERT(Util::MapContains(tool_name_map_, name));
    auto tool = tool_name_map_.at(name);
    ASSERT(tool);
    ASSERT(! tool->IsSpecialized());
    return tool;
}

void ToolManager::UseTool_(const ToolPtr &tool, const Selection &sel) {
    ASSERT(tool);

    KLOG('T', "Now using " << tool->GetTypeName());

    if (sel.HasAny()) {
        // Detach anything that might already be attached.
        DetachToolFromModel_(*sel.GetPrimary().GetModel());

        // Attach the new tool to it.
        AttachToolToModel_(tool, sel, 0);

        // Add observers so that dragging the tool affects the visibility of
        // PassiveTools.
        tool->GetDragStarted().AddObserver(
            this, [&](Tool &tool) { ToolDragStarted_(tool); });
        tool->GetDragEnded().AddObserver(
            this, [&](Tool &tool) { ToolDragEnded_(tool); });
    }

    if (tool->IsSpecialized()) {
        ASSERT(is_using_specialized_tool_);
        current_specialized_tool_ = tool;
    }
    else {
        is_using_specialized_tool_ = false;
        current_general_tool_ = tool;
    }
}

void ToolManager::AttachToolToModel_(const ToolPtr &tool, const Selection &sel,
                                     size_t index) {
    // The Tool should not be attached to anything.
    ASSERT(! tool->GetModelAttachedTo());

    ASSERT(index < sel.GetCount());
    const ModelPtr model = sel.GetPaths()[index].GetModel();
    ASSERT(model);

    KLOG('T', "Attaching " << tool->GetName() << " to " << model->GetDesc());

    // Attach the new Tool after reparenting it.
    parent_node_->AddChild(tool);
    tool->AttachToSelection(sel, index);
    active_tool_map_[model.get()]= tool;

    // Add a listener to detect bounds changes.
    model->GetChanged().AddObserver(
        this, [&, model](SG::Change change, const SG::Object &){
            ModelChanged_(model, change); });
}

void ToolManager::DetachToolFromModel_(Model &model) {
    // Find the attached Tool, if there is one.
    auto it = active_tool_map_.find(&model);
    if (it == active_tool_map_.end())
        return;
    const ToolPtr tool = it->second;

    KLOG('T', "Detaching " << tool->GetName() << " from " << model.GetDesc());

    tool->DetachFromSelection();
    parent_node_->RemoveChild(tool);
    active_tool_map_.erase(it);

    // Remove the observer from the Model.
    model.GetChanged().RemoveObserver(this);

    if (PassiveToolPtr passive_tool = Util::CastToDerived<PassiveTool>(tool)) {
        passive_tool_manager_->Release<PassiveTool>(passive_tool);
    }
    else {
        // Remove listeners from any other Tool.
        tool->GetDragStarted().RemoveObserver(this);
        tool->GetDragEnded().RemoveObserver(this);
    }
}

ToolPtr ToolManager::GetPreviousOrNextGeneralTool_(const Selection &sel,
                                                   bool is_next) {
    ASSERT(! is_using_specialized_tool_);
    ASSERT(sel.HasAny());

    // Find the current tool in the general_tools_ vector.
    auto it = std::find(general_tools_.begin(), general_tools_.end(),
                        current_general_tool_);
    ASSERT(it != general_tools_.end());

    // Loop through the general_tools_ vector for one that works.
    while (true) {
        if (is_next) {
            if (++it == general_tools_.end())
                it = general_tools_.begin();
        }
        else {
            if (it == general_tools_.begin())
                it = general_tools_.end();
            --it;
        }
        if ((*it)->CanBeUsedFor(sel))
            return *it;
    }
    ASSERTM(false, "Could not find a general Tool");
    return ToolPtr();
}

void ToolManager::ModelChanged_(const ModelPtr &model, SG::Change change) {
    // Any non-appearance SG::Change will likely change the Bounds, so
    // reattach. However, if a Tool is actively dragging, it is likely the
    // reason for the change, so do not reattach.
    if (! is_tool_dragging_ && change != SG::Change::kAppearance) {
        ASSERT(Util::MapContains(active_tool_map_, model.get()));
        active_tool_map_.at(model.get())->ReattachToSelection();
    }
}

void ToolManager::ToolDragStarted_(Tool &dragged_tool) {
    for (auto &tool: Util::GetValues(active_tool_map_)) {
        if (Util::IsA<PassiveTool>(tool))
            tool->SetEnabled(false);
    }
    is_tool_dragging_ = true;
}

void ToolManager::ToolDragEnded_(Tool &dragged_tool) {
    is_tool_dragging_ = false;
    for (auto &tool: Util::GetValues(active_tool_map_)) {
        if (Util::IsA<PassiveTool>(tool))
            tool->SetEnabled(true);
    }
}

void ToolManager::TargetActivated_(bool is_activation) {
    const bool is_shown = ! is_activation;
    for (auto &tool: Util::GetValues(active_tool_map_))
        tool->SetEnabled(is_shown);
}
