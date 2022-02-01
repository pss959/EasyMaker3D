#include "Managers/TargetManager.h"

#include <functional>
#include <ion/math/vectorutils.h>

#include "Defaults.h"
#include "Math/Types.h"
#include "Parser/Registry.h"
#include "Util/Assert.h"

TargetManager::TargetManager(const CommandManagerPtr &command_manager) :
    command_manager_(command_manager) {
}

void TargetManager::SetSceneContext(const SceneContextPtr &context) {
    scene_context_ = context;
    coord_conv_.SetStagePath(scene_context_->path_to_stage);
}

void TargetManager::InitTargets(const PointTargetWidgetPtr &ptw,
                                const EdgeTargetWidgetPtr &etw) {
    point_target_widget_ = ptw;
    edge_target_widget_  = etw;

    // Set up callbacks.
    point_target_widget_->GetActivation().AddObserver(
        this, [this](Widget &, bool is_activation){
            PointActivated_(is_activation); });
    point_target_widget_->GetChanged().AddObserver(
        this, [this](Widget &){ PointChanged_(); });
    edge_target_widget_->GetActivation().AddObserver(
        this, [this](Widget &, bool is_activation){
            EdgeActivated_(is_activation); });
    edge_target_widget_->GetChanged().AddObserver(
        this, [this](Widget &){ EdgeChanged_(); });

    // Turn off targets to start.
    if (IsPointTargetVisible())
        TogglePointTarget();
    if (IsEdgeTargetVisible())
        ToggleEdgeTarget();
}

bool TargetManager::IsPointTargetVisible() {
    return point_target_widget_ &&
        point_target_widget_->IsEnabled(SG::Node::Flag::kTraversal);
}

bool TargetManager::IsEdgeTargetVisible() {
    return edge_target_widget_ &&
        edge_target_widget_->IsEnabled(SG::Node::Flag::kTraversal);
}

bool TargetManager::TogglePointTarget() {
    ASSERT(point_target_widget_);
    const bool new_state = ! IsPointTargetVisible();
    point_target_widget_->SetEnabled(SG::Node::Flag::kTraversal, new_state);
    return new_state;
}

bool TargetManager::ToggleEdgeTarget() {
    ASSERT(edge_target_widget_);
    const bool new_state = ! IsEdgeTargetVisible();
    edge_target_widget_->SetEnabled(SG::Node::Flag::kTraversal, new_state);
    return new_state;
}

void TargetManager::StartSnapping() {
    // Nothing to do.
}

void TargetManager::EndSnapping() {
    point_target_widget_->ShowSnapFeedback(coord_conv_, false);
    edge_target_widget_->ShowSnapFeedback(coord_conv_, false);
}

bool TargetManager::SnapToPoint(const Point3f &start_pos,
                                Vector3f &motion_vec) {
    bool is_snapped = false;

    if (IsPointTargetVisible()) {
        const Plane target_plane(GetPointTarget().GetPosition(), motion_vec);
        const Point3f cur_pos  = start_pos + motion_vec;
        const Point3f plane_pt = target_plane.ProjectPoint(cur_pos);
        if (ion::math::Distance(cur_pos, plane_pt) <=
            Defaults::kSnapPointTolerance) {
            is_snapped = true;
            motion_vec = plane_pt - start_pos;
        }
    }
    point_target_widget_->SetSnapFeedbackPoint(start_pos + motion_vec);
    point_target_widget_->ShowSnapFeedback(coord_conv_, is_snapped);
    return is_snapped;
}

void TargetManager::PointActivated_(bool is_activation) {
    target_activation_.Notify(is_activation);

    if (is_activation) {
        // Create the command to move the target.
        ASSERT(! point_command_);
        point_command_ =
            Parser::Registry::CreateObject<ChangePointTargetCommand>();
    }
    else {
        ASSERT(point_command_);
        if (point_command_->GetNewTarget()->WasAnyFieldSet())
            command_manager_->AddAndDo(point_command_);
        point_command_.reset();
        point_target_widget_->ShowSnapFeedback(coord_conv_, false);
    }
}

void TargetManager::PointChanged_() {
    ASSERT(point_command_);
    point_command_->GetNewTarget()->CopyFrom(GetPointTarget());
}

void TargetManager::EdgeActivated_(bool is_activation) {
    target_activation_.Notify(is_activation);

    if (is_activation) {
        // Create the command to move the target.
        ASSERT(! edge_command_);
        edge_command_ =
            Parser::Registry::CreateObject<ChangeEdgeTargetCommand>();
    }
    else {
        ASSERT(edge_command_);
        if (edge_command_->GetNewTarget()->WasAnyFieldSet())
            command_manager_->AddAndDo(edge_command_);
        edge_command_.reset();
        edge_target_widget_->ShowSnapFeedback(coord_conv_, false);
    }
}

void TargetManager::EdgeChanged_() {
    ASSERT(edge_command_);
    edge_command_->GetNewTarget()->CopyFrom(GetEdgeTarget());
}
