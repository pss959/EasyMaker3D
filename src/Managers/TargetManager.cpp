#include "Managers/TargetManager.h"

#include <functional>

#include "Parser/Registry.h"
#include "Util/Assert.h"

TargetManager::TargetManager(const CommandManagerPtr &command_manager) :
    command_manager_(command_manager) {
}

void TargetManager::InitPointTarget(const PointTargetWidgetPtr &widget) {
    point_target_widget_ = widget;

    // Set up callbacks.
    point_target_widget_->GetActivation().AddObserver(
        this, [this](Widget &, bool is_activation){
            PointActivated_(is_activation); });
    point_target_widget_->GetChanged().AddObserver(
        this, [this](Widget &){ PointChanged_(); });

    // Turn off targets to start.
    if (IsPointTargetVisible())
        TogglePointTarget();

#if XXXX
    edge_target_widget_->GetActivation().AddObserver(this, EdgeActivated_);
    edge_target_widget_->GetMoved().AddObserver(this, EdgeMoved_);
    edge_target_widget_->GetClicked().AddObserver(this, EdgeClicked_);
#endif
}

bool TargetManager::IsPointTargetVisible() {
    return point_target_widget_ &&
        point_target_widget_->IsEnabled(SG::Node::Flag::kTraversal);
}

bool TargetManager::IsEdgeTargetVisible() {
#if XXXX
    return edge_target_widget_ &&
        edge_target_widget_->IsEnabled(SG::Node::Flag::kTraversal);
#else
    return false;  // XXXX
#endif
}

bool TargetManager::TogglePointTarget() {
    ASSERT(point_target_widget_);
    const bool new_state = ! IsPointTargetVisible();
    point_target_widget_->SetEnabled(SG::Node::Flag::kTraversal, new_state);
    return new_state;
}

bool TargetManager::ToggleEdgeTarget() {
#if XXXX
    ASSERT(edge_target_widget_);
    const bool new_state = ! IsEdgeTargetVisible();
    edge_target_widget_->SetEnabled(SG::Node::Flag::kTraversal, new_state);
    return new_state;
#endif
    return false; // XXXX
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
        point_target_widget_->ShowSnapFeedback(false);
    }
}

void TargetManager::PointChanged_() {
    ASSERT(point_command_);
    point_command_->GetNewTarget()->CopyFrom(GetPointTarget());
}
