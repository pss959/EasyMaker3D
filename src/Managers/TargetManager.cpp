#include "Managers/TargetManager.h"

#include <functional>

TargetManager::TargetManager(const CommandManagerPtr &command_manager) :
    command_manager_(command_manager) {
}

void TargetManager::InitPointTarget(const PointTargetWidgetPtr &widget) {
    point_target_widget_ = widget;

    // Set up callbacks.
    point_target_widget_->GetActivation().AddObserver(
        this, [this](Widget &, bool is_activation){
            PointActivated(is_activation); });
    point_target_widget_->GetMoved().AddObserver(
        this, [this](Widget &){ PointMoved(); });

#if XXXX
    edge_target_widget_->GetActivation().AddObserver(this, EdgeActivated);
    edge_target_widget_->GetMoved().AddObserver(this, EdgeMoved);
    edge_target_widget_->GetClicked().AddObserver(this, EdgeClicked);
#endif
}

void TargetManager::PointActivated(bool is_activation) {
    std::cerr << "XXXX PointActivated is_activation = " << is_activation << "\n";
}

void TargetManager::PointMoved() {
    std::cerr << "XXXX PointMoved\n";
}
