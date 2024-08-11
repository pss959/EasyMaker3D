//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Managers/FeedbackManager.h"

#include <algorithm>
#include <unordered_map>

#include "SG/CoordConv.h"
#include "Util/KLog.h"

void FeedbackManager::SetParentNodes(const SG::NodePtr &world_parent,
                                     const SG::NodePtr &stage_parent) {
    ASSERT(world_parent);
    ASSERT(stage_parent);
    world_parent_node_ = world_parent;
    stage_parent_node_ = stage_parent;
}

void FeedbackManager::SetSceneBoundsFunc(const SceneBoundsFunc &func) {
    ASSERT(func);
    scene_bounds_func_ = func;

    // Update any active instances.
    for (auto &item: unkeyed_active_instances_)
        item->SetSceneBoundsFunc(func);
    for (auto &item: keyed_active_instances_)
        item.second->SetSceneBoundsFunc(func);
}

void FeedbackManager::ActivateInstance_(const FeedbackPtr &instance) {
    ASSERT(instance);
    KLOG('D', "Activating instance " << instance->GetDesc());
    instance->Activate();
    instance->SetSceneBoundsFunc(scene_bounds_func_);
    instance->SetObjectToWorldMatrix(
        SG::CoordConv(path_to_stage_).GetObjectToRootMatrix());

    // Add the instance to the correct parent.
    auto &parent = instance->IsInWorldCoordinates() ?
        world_parent_node_ : stage_parent_node_;
    parent->AddChild(instance);

    unkeyed_active_instances_.push_back(instance);
}

void FeedbackManager::DeactivateInstance_(const FeedbackPtr &instance) {
    ASSERT(instance);

    // Remove from active instances.
    auto it = std::find(unkeyed_active_instances_.begin(),
                        unkeyed_active_instances_.end(), instance);
    ASSERT(it != unkeyed_active_instances_.end());
    unkeyed_active_instances_.erase(it);

    KLOG('D', "Deactivating instance " << instance->GetDesc());
    instance->Deactivate();

    if (instance->IsInWorldCoordinates())
        world_parent_node_->RemoveChild(instance);
    else
        stage_parent_node_->RemoveChild(instance);
}

void FeedbackManager::AddKeyedActiveInstance_(const Str &key,
                                              const FeedbackPtr &instance) {
    ASSERTM(! keyed_active_instances_.contains(key), key);
    KLOG('D', "Adding active instance " << instance->GetDesc()
         << " with key " << key);
    keyed_active_instances_[key] = instance;
}

void FeedbackManager::RemoveKeyedActiveInstance_(const Str &key,
                                                 const FeedbackPtr &instance) {
    ASSERTM(keyed_active_instances_.contains(key), key);
    KLOG('D', "Removing active instance " << instance->GetDesc()
         << " with key " << key);
    keyed_active_instances_.erase(key);
}

FeedbackPtr FeedbackManager::FindKeyedActiveInstance_(const Str &key) {
    const auto it = keyed_active_instances_.find(key);
    if (it != keyed_active_instances_.end())
        return it->second;
    return FeedbackPtr();
}
