#include "Managers/FeedbackManager.h"

#include <forward_list>
#include <typeindex>
#include <unordered_map>

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "App/CoordConv.h"
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

    // Add the instance to the correct parent.
    auto &parent = instance->IsInWorldCoordinates() ?
        world_parent_node_ : stage_parent_node_;
    parent->AddChild(instance);

    // Feedback text should always face the camera and should be at a
    // reasonable size.
    SetTextScaleAndRotation_(*instance);
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

void FeedbackManager::SetTextScaleAndRotation_(Feedback &instance) {
    // Compute a scale to make the text a reasonable size in world coordinates
    // and compute the rotation that brings the XY plane in stage coordinates
    // to the XY plane in world coordinates so the text faces the camera.
    float     scale = 1;
    Rotationf rotation;
    if (! instance.IsInWorldCoordinates()) {
        // Compute a relative size in world coordinates.
        using ion::math::Length;
        using ion::math::Normalized;

        const Matrix4f to_world =
            CoordConv(path_to_stage_).GetObjectToRootMatrix();

        const float kFeedbackTextScale = 1.2f;
        const float size = Length(to_world * Normalized(Vector3f(1, 1, 1)));
        scale = kFeedbackTextScale / size;

        const Vector3f stage_z_axis = Vector3f::AxisZ();
        const Vector3f world_z_axis = to_world * stage_z_axis;
        rotation = Rotationf::RotateInto(world_z_axis, stage_z_axis);
    }
    instance.SetTextScaleAndRotation(scale, rotation);
}
