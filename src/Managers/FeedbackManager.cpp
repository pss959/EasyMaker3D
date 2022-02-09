#include "Managers/FeedbackManager.h"

#include <forward_list>
#include <memory>
#include <typeindex>
#include <unordered_map>

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

void FeedbackManager::ClearTemplates() {
    template_map_.clear();
    available_instances_.clear();
}

void FeedbackManager::ActivateInstance_(const FeedbackPtr &instance) {
    ASSERT(instance);
    instance->Activate();
    instance->SetSceneBoundsFunc(scene_bounds_func_);

    if (instance->IsInWorldCoordinates())
        world_parent_node_->AddChild(instance);
    else
        stage_parent_node_->AddChild(instance);
}

void FeedbackManager::DeactivateInstance_(const FeedbackPtr &instance) {
    ASSERT(instance);
    instance->Deactivate();

    if (instance->IsInWorldCoordinates())
        world_parent_node_->RemoveChild(instance);
    else
        stage_parent_node_->RemoveChild(instance);
}
