#include "Managers/FeedbackManager.h"

#include <forward_list>
#include <typeindex>
#include <unordered_map>

#include "CoordConv.h"
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

    // Stores the rotation that brings the XY plane in stage coordinates to the
    // XY plane in world coordinates. This rotation is used to orient text
    // feedback.
    Rotationf text_rotation;
    if (instance->IsInWorldCoordinates()) {
        world_parent_node_->AddChild(instance);
        // Leave text_rotation as identity.
    }
    else {
        stage_parent_node_->AddChild(instance);
        const Vector3f stage_z_axis = Vector3f::AxisZ();
        const Vector3f world_z_axis =
            CoordConv(path_to_stage_).ObjectToRoot(stage_z_axis);
        text_rotation = Rotationf::RotateInto(world_z_axis, stage_z_axis);
    }
    instance->SetTextRotation(text_rotation);
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
    ASSERTM(! Util::MapContains(active_instances_, key), key);
    KLOG('d', "Adding active instance " << instance->GetDesc()
         << " with key " << key);
    active_instances_[key] = instance;
}

void FeedbackManager::RemoveActiveInstance_(const std::string &key,
                                            const FeedbackPtr &instance) {
    ASSERTM(Util::MapContains(active_instances_, key), key);
    KLOG('d', "Removing active instance " << instance->GetDesc()
         << " with key " << key);
    active_instances_.erase(key);
}

FeedbackPtr FeedbackManager::FindActiveInstance_(const std::string &key) {
    const auto it = active_instances_.find(key);
    if (it != active_instances_.end())
        return it->second;
    return FeedbackPtr();
}
