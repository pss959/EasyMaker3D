#include "Managers/FeedbackManager.h"

#include <forward_list>
#include <typeindex>
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
}

void FeedbackManager::DeactivateInstance_(const FeedbackPtr &instance) {
    ASSERT(instance);
    KLOG('D', "Deactivating instance " << instance->GetDesc());
    instance->Deactivate();

    if (instance->IsInWorldCoordinates())
        world_parent_node_->RemoveChild(instance);
    else
        stage_parent_node_->RemoveChild(instance);
}

void FeedbackManager::AddActiveInstance_(const Str &key,
                                         const FeedbackPtr &instance) {
    ASSERTM(! active_instances_.contains(key), key);
    KLOG('D', "Adding active instance " << instance->GetDesc()
         << " with key " << key);
    active_instances_[key] = instance;
}

void FeedbackManager::RemoveActiveInstance_(const Str &key,
                                            const FeedbackPtr &instance) {
    ASSERTM(active_instances_.contains(key), key);
    KLOG('D', "Removing active instance " << instance->GetDesc()
         << " with key " << key);
    active_instances_.erase(key);
}

FeedbackPtr FeedbackManager::FindActiveInstance_(const Str &key) {
    const auto it = active_instances_.find(key);
    if (it != active_instances_.end())
        return it->second;
    return FeedbackPtr();
}
