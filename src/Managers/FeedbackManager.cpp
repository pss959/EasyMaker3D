#include "Managers/FeedbackManager.h"

#include <forward_list>
#include <memory>
#include <typeindex>
#include <unordered_map>

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
    KLOG('d', "Activating instance " << instance->GetDesc());
    instance->Activate();
    instance->SetSceneBoundsFunc(scene_bounds_func_);

    if (instance->IsInWorldCoordinates())
        world_parent_node_->AddChild(instance);
    else
        stage_parent_node_->AddChild(instance);
}

void FeedbackManager::DeactivateInstance_(const FeedbackPtr &instance) {
    ASSERT(instance);
    KLOG('d', "Deactivating instance " << instance->GetDesc());
    instance->Deactivate();

    if (instance->IsInWorldCoordinates())
        world_parent_node_->RemoveChild(instance);
    else
        stage_parent_node_->RemoveChild(instance);
}

void FeedbackManager::AddActiveInstance_(const std::string &key,
                                         const FeedbackPtr &instance) {
    ASSERT(! Util::MapContains(active_instances_, key));
    active_instances_[key] = instance;
}

void FeedbackManager::RemoveActiveInstance_(const std::string &key,
                                            const FeedbackPtr &instance) {
    ASSERT(Util::MapContains(active_instances_, key));
    active_instances_.erase(key);
}

FeedbackPtr FeedbackManager::FindActiveInstance_(const std::string &key) {
    const auto it = active_instances_.find(key);
    if (it != active_instances_.end())
        return it->second;
    return FeedbackPtr();
}
