#include "Managers/TargetManager.h"

#include <functional>
#include <limits>

#include <ion/math/vectorutils.h>

#include "Commands/ChangeEdgeTargetCommand.h"
#include "Commands/ChangePointTargetCommand.h"
#include "CoordConv.h"
#include "Defaults.h"
#include "Managers/CommandManager.h"
#include "Math/Linear.h"
#include "Math/Types.h"
#include "Parser/Registry.h"
#include "Targets/EdgeTarget.h"
#include "Targets/PointTarget.h"
#include "Util/Assert.h"
#include "Widgets/EdgeTargetWidget.h"
#include "Widgets/PointTargetWidget.h"

TargetManager::TargetManager(const CommandManagerPtr &command_manager) :
    command_manager_(command_manager) {
}

void TargetManager::SetPathToStage(const SG::NodePath &path) {
    path_to_stage_ = path;
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
    edge_target_widget_->GetClicked().AddObserver(
        this, [this](const ClickInfo &){ EdgeClicked_(); });

    // Turn off targets to start.
    SetPointTargetVisible(false);
    SetEdgeTargetVisible(false);
}

bool TargetManager::IsPointTargetVisible() {
    return point_target_widget_ &&
        point_target_widget_->IsEnabled();
}

bool TargetManager::IsEdgeTargetVisible() {
    return edge_target_widget_ &&
        edge_target_widget_->IsEnabled();
}

void TargetManager::SetPointTargetVisible(bool visible) {
    ASSERT(point_target_widget_);
    point_target_widget_->SetEnabled(visible);
}

void TargetManager::SetEdgeTargetVisible(bool visible) {
    ASSERT(edge_target_widget_);
    edge_target_widget_->SetEnabled(visible);
}

const PointTarget & TargetManager::GetPointTarget() const {
    return point_target_widget_->GetPointTarget();
}

void TargetManager::SetPointTarget(const PointTarget &target) {
    point_target_widget_->SetPointTarget(target);
}

const EdgeTarget & TargetManager::GetEdgeTarget() const {
    return edge_target_widget_->GetEdgeTarget();
}

void TargetManager::SetEdgeTarget(const EdgeTarget &target) {
    edge_target_widget_->SetEdgeTarget(target);
}

void TargetManager::StartSnapping() {
    // Nothing to do.
}

void TargetManager::EndSnapping() {
    CoordConv stage_cc(path_to_stage_);
    ShowSnapFeedback_(*point_target_widget_, false);
    ShowSnapFeedback_(*edge_target_widget_,  false);
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
    ShowSnapFeedback_(*point_target_widget_, is_snapped);
    return is_snapped;
}

bool TargetManager::SnapToDirection(const Vector3f &dir, Rotationf &rot) {
    if (! IsPointTargetVisible())
        return false;

    const Vector3f unit_dir = ion::math::Normalized(dir);

    auto test_dir = [&rot, &unit_dir](const Vector3f &dir_to_test){
        const Rotationf r = Rotationf::RotateInto(unit_dir, dir_to_test);
        if (std::abs(RotationAngle(r).Degrees()) <=
            Defaults::kSnapDirectionTolerance) {
            rot = r;
            return true;
        }
        else {
            return false;
        }
    };

    // Check angles in both directions.
    const Vector3f &target_dir = GetPointTarget().GetDirection();
    return test_dir(target_dir) || test_dir(-target_dir);
}

bool TargetManager::SnapToLength(float length) {
    float diff;
    const bool is_snapped = SnapToLengthWithDiff_(length, diff);
    ShowSnapFeedback_(*edge_target_widget_, is_snapped);
    return is_snapped;
}

Dimensionality TargetManager::SnapToLength(const Dimensionality &dims,
                                           const Vector3f &vec) {
    Dimensionality snapped_dims;
    float min_diff = std::numeric_limits<float>::max();
    for (int dim = 0; dim < 3; ++dim) {
        if (dims.HasDimension(dim)) {
            float diff;
            if (SnapToLengthWithDiff_(vec[dim], diff) && diff <= min_diff) {
                snapped_dims.AddDimension(dim);
                min_diff = diff;
            }
        }
    }
    ShowSnapFeedback_(*edge_target_widget_, snapped_dims.GetCount() > 0);
    return snapped_dims;
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
        ShowSnapFeedback_(*point_target_widget_, false);
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
        ShowSnapFeedback_(*edge_target_widget_, false);
    }
}

void TargetManager::EdgeChanged_() {
    ASSERT(edge_command_);
    edge_command_->GetNewTarget()->CopyFrom(GetEdgeTarget());
}

void TargetManager::EdgeClicked_() {
    ASSERT(! edge_command_);

    // Create a command to switch the direction of the EdgeTarget.
    edge_command_ = Parser::Registry::CreateObject<ChangeEdgeTargetCommand>();
    const auto &cur_target = GetEdgeTarget();
    edge_command_->GetNewTarget()->SetPositions(cur_target.GetPosition1(),
                                                cur_target.GetPosition0());
    command_manager_->AddAndDo(edge_command_);
    edge_command_.reset();
}

bool TargetManager::SnapToLengthWithDiff_(float length, float &diff) {
    if (IsEdgeTargetVisible()) {
        diff = std::fabs(length - GetEdgeTarget().GetLength());
        if (diff <= Defaults::kSnapLengthTolerance)
            return true;
    }
    // No snapping.
    diff = 0;
    return false;
}

void TargetManager::ShowSnapFeedback_(TargetWidgetBase &widget,
                                      bool is_snapped) {
    widget.ShowSnapFeedback(CoordConv(path_to_stage_), is_snapped);
}
