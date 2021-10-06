﻿#include "Managers/ToolManager.h"

#include "Assert.h"
#include "Tools/PassiveTool.h"
#include "Util/General.h"

#if XXXX
ToolManager::ToolManager(TargetManager &target_manager) {
    // Attach a callback to the TargetManager to turn off active tools while
    // the target is being dragged so the tool geometry does not interfere with
    // target placement.
    target_manager.GetActivation().AddObserver(TargetActivated_);
}
#endif

void ToolManager::SetParentNode(const SG::NodePtr &parent_node) {
    parent_node_ = parent_node;
}

void ToolManager::AddGeneralTool(const GeneralToolPtr &tool) {
    ASSERT(! Util::MapContains(general_tool_map_, tool->GetTypeName()));
    general_tool_map_[tool->GetTypeName()] = tool;
    general_tools_.push_back(tool);
}

void ToolManager::AddSpecializedTool(const SpecializedToolPtr &tool) {
    specialized_tool_map_[tool->GetTypeName()] = tool;
}

void ToolManager::SetDefaultGeneralTool(const GeneralToolPtr &tool) {
    ASSERT(! current_general_tool_);
    ASSERT(Util::MapContains(general_tool_map_, tool->GetTypeName()));
    current_general_tool_ = tool;
}

bool ToolManager::CanUseGeneralTool(const std::string &name,
                                    const Selection &sel) {
    // There has to be at least one Model selected and the correct tool has to
    // support attaching to the selection.
    ASSERT(Util::MapContains(general_tool_map_, name));
    GeneralToolPtr tool = general_tool_map_.at(name);
    ASSERT(tool);
    return sel.HasAny() && tool->CanBeUsedFor(sel);
}

bool ToolManager::CanUseSpecializedTool(const Selection &sel) {
    return GetSpecializedToolForSelection(sel).get();
}

SpecializedToolPtr ToolManager::GetSpecializedToolForSelection(
    const Selection &sel) {
    // Find a SpecializedTool that can attach to the selection.
    auto it = std::find_if(specialized_tool_map_.begin(),
                           specialized_tool_map_.end(),
                           [sel](const SpecializedMap_::value_type &v){
                              return v.second->CanBeUsedFor(sel); });
    return it == specialized_tool_map_.end() ?
        SpecializedToolPtr() : it->second;
}

void ToolManager::UseGeneralTool(const std::string &name,
                                 const Selection &sel) {
    ASSERT(Util::MapContains(general_tool_map_, name));
    GeneralToolPtr tool = general_tool_map_.at(name);
    ASSERT(tool);
    UseTool_(tool, sel);
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

std::vector<std::string> ToolManager::GetSpecializedToolNames() {
    ASSERT(! specialized_tool_map_.empty());
    return Util::GetKeys(specialized_tool_map_);
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
        is_using_specialized_tool_ = false;
        if (! current_general_tool_->CanBeUsedFor(sel))
            current_general_tool_ = GetPreviousOrNextGeneralTool_(sel, true);
        tool = current_general_tool_;
    }

    // Attach the proper tool for the primary selection.
    ASSERT(tool);
    UseTool_(tool, sel);

#if XXXX
    // Attach a PassiveTool to all secondary selections.
    foreach (Model model in sel.Iterator()) {
        if (model != sel.GetPrimary()) {
            Tool passiveTool = _passiveToolHelper.Acquire();
            Assert.IsNotNull(passiveTool);
            AttachToolToModel(passiveTool, model, sel);
        }
    }
#endif
}

void ToolManager::DetachTools(const Selection &sel) {
    for (auto &path: sel.GetPaths())
        DetachToolFromModel_(*path.GetModel());
}

void ToolManager::DetachAllTools() {
    // Save pointers to all attached Models from the map; the detaching code
    // removes entries from the map.
    const auto models = Util::GetKeys(tool_map_);

    for (auto &model: models)
        DetachToolFromModel_(*model);
}

void ToolManager::ReattachTools() {
    for (auto &tool: Util::GetValues(tool_map_))
        tool->ReattachToSelection();
}

ToolPtr ToolManager::GetAttachedTool(const ModelPtr &model) const {
    return Util::MapContains(tool_map_, model.get()) ?
        tool_map_.at(model.get()) : ToolPtr();
}

void ToolManager::UseTool_(const ToolPtr &tool, const Selection &sel) {
    ASSERT(tool);

    if (sel.HasAny()) {
        const ModelPtr primary = sel.GetPrimary().GetModel();

        // Detach anything that might already be attached.
        DetachToolFromModel_(*primary);

        // Attach the new tool to it.
        AttachToolToModel_(tool, primary, sel);
    }

    // Add observers so that dragging the tool affects the visibility of
    // PassiveTools.
    tool->GetDragStarted().AddObserver(this,
                                       std::bind(&ToolManager::ToolDragStarted_,
                                                 this, std::placeholders::_1));
    tool->GetDragEnded().AddObserver(this,
                                     std::bind(&ToolManager::ToolDragEnded_,
                                               this, std::placeholders::_1));

    if (Util::IsA<SpecializedTool>(tool)) {
        ASSERT(is_using_specialized_tool_);
        current_specialized_tool_ = Util::CastToDerived<SpecializedTool>(tool);
    }
    else {
        ASSERT(Util::IsA<GeneralTool>(tool));
        is_using_specialized_tool_ = false;
        current_general_tool_ = Util::CastToDerived<GeneralTool>(tool);
    }
}

void ToolManager::AttachToolToModel_(const ToolPtr &tool, const ModelPtr &model,
                                     const Selection &sel) {
    // The Tool should not be attached to anything.
    ASSERT(! tool->GetPrimaryModel());

    // Attach the new Tool after reparenting it.
    parent_node_->AddChild(tool);
    tool->AttachToSelection(sel);
    tool_map_[model.get()]= tool;

    // Add a listener to detect bounds changes.
    model->GetChanged().AddObserver(
        this, std::bind(&ToolManager::ModelChanged_,
                        this, model, std::placeholders::_1));
}

void ToolManager::DetachToolFromModel_(Model &model) {
    // Find the attached Tool, if there is one.
    auto it = tool_map_.find(&model);
    if (it == tool_map_.end())
        return;
    const ToolPtr tool = it->second;

    tool->DetachFromSelection();
    parent_node_->RemoveChild(tool);
    tool_map_.erase(it);

    // Remove the observer from the Model.
    model.GetChanged().RemoveObserver(this);

    PassiveToolPtr passive_tool = Util::CastToDerived<PassiveTool>(tool);
    if (passive_tool) {
        // XXXX passive_tool_helper_.Release(passive_tool);
    }
    else {
        // Remove listeners from any other Tool.
        tool->GetDragStarted().RemoveObserver(this);
        tool->GetDragEnded().RemoveObserver(this);
    }
}

GeneralToolPtr ToolManager::GetPreviousOrNextGeneralTool_(const Selection &sel,
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
    ASSERTM(false, "Could not find a GeneralTool");
    return GeneralToolPtr();
}

void ToolManager::ModelChanged_(const ModelPtr &model, SG::Change change) {
    // Any non-appearance SG::Change will likely change the Bounds, so
    // reattach. However, if a Tool is actively dragging, it is likely the
    // reason for the change, so do not reattach.
    if (! is_tool_dragging_ && change != SG::Change::kAppearance) {
        ASSERT(Util::MapContains(tool_map_, model.get()));
        tool_map_.at(model.get())->ReattachToSelection();
    }
}

void ToolManager::ToolDragStarted_(Tool &dragged_tool) {
    for (auto &tool: Util::GetValues(tool_map_)) {
        if (Util::IsA<PassiveTool>(tool))
            tool->SetEnabled(SG::Node::Flag::kTraversal, false);
    }
    is_tool_dragging_ = true;
}

void ToolManager::ToolDragEnded_(Tool &dragged_tool) {
    is_tool_dragging_ = false;
    for (auto &tool: Util::GetValues(tool_map_)) {
        if (Util::IsA<PassiveTool>(tool))
            tool->SetEnabled(SG::Node::Flag::kTraversal, true);
    }
}

void ToolManager::TargetActivated_(bool is_activation) {
    const bool is_shown = ! is_activation;
    for (auto &tool: Util::GetValues(tool_map_))
        tool->SetEnabled(SG::Node::Flag::kTraversal, is_shown);
}
