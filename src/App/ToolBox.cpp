//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "App/ToolBox.h"

#include <vector>

#include "Managers/TargetManager.h"
#include "Models/Model.h"
#include "Parser/InstanceStore.h"
#include "Tools/PassiveTool.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/KLog.h"

ToolBox::ToolBox() : passive_tool_store_(new Parser::InstanceStore) {}

void ToolBox::SetTargetManager(TargetManager &target_manager) {
    // Attach a callback to the TargetManager to turn off active tools while
    // the target is being dragged so the tool geometry does not interfere with
    // target placement.
    target_manager.GetTargetActivation().AddObserver(
        this, [&](bool is_activation){ TargetActivated_(is_activation); });
}

void ToolBox::SetParentNode(const SG::NodePtr &parent_node) {
    parent_node_ = parent_node;
}

void ToolBox::AddTools(const std::vector<ToolPtr> &tools) {
    // This is used as the completion function for all specialized Tools. This
    // just toggles back to an appropriate general Tool.
    auto completion_func = [&](){
        ToggleSpecializedTool(GetCurrentTool()->GetSelection());
    };

    for (auto &tool: tools) {
        const Str &type_name = tool->GetTypeName();

        // Special case for the PassiveTool.
        if (PassiveToolPtr pt = std::dynamic_pointer_cast<PassiveTool>(tool)) {
            KLOG('T', "Adding PassiveTool");
            passive_tool_store_->AddOriginal<PassiveTool>(pt);
        }
        else {
            ASSERTM(! tool_name_map_.contains(type_name), type_name);
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
    ASSERT(passive_tool_store_->HasOriginal<PassiveTool>());
}

void ToolBox::SetDefaultGeneralTool(const Str &name) {
    ASSERT(! current_general_tool_);
    auto tool = tool_name_map_.at(name);
    ASSERT(tool);
    ASSERT(! tool->IsSpecialized());
    default_general_tool_ = tool;
    current_general_tool_ = default_general_tool_;
}

void ToolBox::ClearTools() {
    Reset();
    tool_name_map_.clear();
    general_tools_.clear();
    specialized_tools_.clear();
    current_general_tool_.reset();
    current_specialized_tool_.reset();
    default_general_tool_.reset();
    is_using_specialized_tool_ = false;
}

void ToolBox::Reset() {
    ResetSession();
    passive_tool_store_->Reset();
}

void ToolBox::ResetSession() {
    // Detach all attached tools
    for (auto &tool: Util::GetValues(active_tool_map_))
        tool->DetachFromSelection();
    active_tool_map_.clear();
    current_general_tool_ = default_general_tool_;
    current_specialized_tool_.reset();
    is_using_specialized_tool_ = false;
}

bool ToolBox::CanUseGeneralTool(const Str &name, const Selection &sel) {
    // There has to be at least one Model selected and the correct tool has to
    // support attaching to the selection.
    return sel.HasAny() && GetGeneralTool_(name)->CanBeUsedFor(sel);
}

bool ToolBox::CanUseSpecializedTool(const Selection &sel) {
    return GetSpecializedToolForSelection(sel).get();
}

ToolPtr ToolBox::GetSpecializedToolForSelection(const Selection &sel) {
    // Find a SpecializedTool that can attach to the selection.
    auto it = std::find_if(specialized_tools_.begin(),
                           specialized_tools_.end(),
                           [sel](const ToolPtr &tool){
                           return tool->CanBeUsedFor(sel); });
    return it != specialized_tools_.end() ? *it : ToolPtr();
}

void ToolBox::UseGeneralTool(const Str &name, const Selection &sel) {
    UseTool_(GetGeneralTool_(name), sel);
}

void ToolBox::UseSpecializedTool(const Selection &sel) {
    if (! is_using_specialized_tool_ && CanUseSpecializedTool(sel))
        ToggleSpecializedTool(sel);
}

void ToolBox::ToggleSpecializedTool(const Selection &sel) {
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

ToolPtr ToolBox::GetCurrentTool() const {
    if (is_using_specialized_tool_)
        return current_specialized_tool_;
    else
        return current_general_tool_;
}

void ToolBox::AttachToSelection(const Selection &sel) {
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
        auto pt = passive_tool_store_->Acquire<PassiveTool>();
        AttachToolToModel_(pt, sel, i);
    }
}

void ToolBox::DetachTools(const Selection &sel) {
    for (auto &path: sel.GetPaths())
        DetachToolFromModel_(*path.GetModel());
}

void ToolBox::ReattachTools() {
    ASSERT(! is_tool_dragging_);

    // Set this flag temporarily so that any changes to a Model caused by
    // reattaching to it do not trigger yet another reattachment.
    is_tool_dragging_ = true;
    for (auto &tool: Util::GetValues(active_tool_map_))
        tool->ReattachToSelection();
    is_tool_dragging_ = false;
}

ToolPtr ToolBox::GetAttachedTool(const ModelPtr &model) const {
    return active_tool_map_.contains(model.get()) ?
        active_tool_map_.at(model.get()) : ToolPtr();
}

const SG::Node * ToolBox::GetGrippableNode() const {
    auto tool = GetCurrentTool();
    return tool ? tool->GetGrippableNode() : nullptr;
}

void ToolBox::UpdateGripInfo(GripInfo &info) {
    // This should not be called unless there is a current Tool.
    ASSERT(GetCurrentTool());

    // If the current Tool is not attached, there is nothing to update.
    auto tool = GetCurrentTool();
    if (tool->GetModelAttachedTo())
        tool->UpdateGripInfo(info);
}

ToolPtr ToolBox::GetGeneralTool_(const Str &name) const {
    ASSERT(tool_name_map_.contains(name));
    auto tool = tool_name_map_.at(name);
    ASSERT(tool);
    ASSERT(! tool->IsSpecialized());
    return tool;
}

void ToolBox::UseTool_(const ToolPtr &tool, const Selection &sel) {
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

void ToolBox::AttachToolToModel_(const ToolPtr &tool, const Selection &sel,
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

void ToolBox::DetachToolFromModel_(Model &model) {
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

    if (PassiveToolPtr passive_tool =
        std::dynamic_pointer_cast<PassiveTool>(tool)) {
        passive_tool_store_->Release<PassiveTool>(passive_tool);
    }
    else {
        // Remove listeners from any other Tool.
        tool->GetDragStarted().RemoveObserver(this);
        tool->GetDragEnded().RemoveObserver(this);
    }
}

ToolPtr ToolBox::GetPreviousOrNextGeneralTool_(const Selection &sel,
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

void ToolBox::ModelChanged_(const ModelPtr &model, SG::Change change) {
    // Any non-appearance SG::Change will likely change the Bounds, so
    // reattach. However, if a Tool is actively dragging, it is likely the
    // reason for the change, so do not reattach.
    if (! is_tool_dragging_ && change != SG::Change::kAppearance) {
        ASSERT(active_tool_map_.contains(model.get()));
        active_tool_map_.at(model.get())->ReattachToSelection();
    }
}

void ToolBox::ToolDragStarted_(Tool &dragged_tool) {
    for (auto &tool: Util::GetValues(active_tool_map_)) {
        if (Util::IsA<PassiveTool>(tool))
            tool->SetEnabled(false);
    }
    is_tool_dragging_ = true;
}

void ToolBox::ToolDragEnded_(Tool &dragged_tool) {
    is_tool_dragging_ = false;
    for (auto &tool: Util::GetValues(active_tool_map_)) {
        if (Util::IsA<PassiveTool>(tool))
            tool->SetEnabled(true);
    }
}

void ToolBox::TargetActivated_(bool is_activation) {
    const bool is_shown = ! is_activation;
    for (auto &tool: Util::GetValues(active_tool_map_))
        tool->SetEnabled(is_shown);
}
