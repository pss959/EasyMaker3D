//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tools/SpinBasedTool.h"

#include <algorithm>
#include <limits>

#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Commands/ChangeSpinCommand.h"
#include "Feedback/AngularFeedback.h"
#include "Feedback/LinearFeedback.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/TargetManager.h"
#include "Math/Linear.h"
#include "Math/Snap3D.h"
#include "Models/ConvertedModel.h"
#include "Place/PointTarget.h"
#include "Place/PrecisionStore.h"
#include "SG/ColorMap.h"
#include "SG/CoordConv.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"
#include "Widgets/SpinWidget.h"

SpinBasedTool::SpinBasedTool() {
}

void SpinBasedTool::CreationDone() {
    Tool::CreationDone();

    if (! IsTemplate()) {
        spin_widget_ = SG::FindTypedNodeUnderNode<SpinWidget>(*this,
                                                              "SpinWidget");

        // Set up callbacks.
        spin_widget_->GetActivation().AddObserver(
            this, [&](Widget &, bool is_act){ Activate_(is_act); });
        spin_widget_->GetSpinChanged().AddObserver(
            this, [&](SpinWidget::ChangeType type){ SpinChanged_(type); });
    }
}

void SpinBasedTool::UpdateGripInfo(GripInfo &info) {
    // If the direction is close to the axis direction (in either direction),
    // use the axis rotator.
    const Vector3f &guide_dir = info.guide_direction;
    const Vector3f &axis      = spin_widget_->GetSpin().axis;
    WidgetPtr       widget;
    if (AreDirectionsClose(guide_dir,  axis, TK::kMaxGripHoverDirAngle) ||
        AreDirectionsClose(guide_dir, -axis, TK::kMaxGripHoverDirAngle)) {
        widget          = spin_widget_->GetSubWidget("Rotator");
        info.guide_type = GripGuideType::kRotation;
    }

    // If the direction is close to perpendicular to the axis direction, use
    // the axis translator.
    else if (AreAlmostPerpendicular(guide_dir, axis,
                                    TK::kMaxGripHoverDirAngle)) {
        widget          = spin_widget_->GetSubWidget("Translator");
        info.guide_type = GripGuideType::kBasic;
    }

    // Otherwise, use the spin ring.
    else {
        widget          = spin_widget_->GetSubWidget("Ring");
        info.guide_type = GripGuideType::kRotation;
    }
    info.widget = std::dynamic_pointer_cast<ClickableWidget>(widget);
    ASSERT(info.widget);
    info.target_point = ToWorld(info.widget, Point3f::Zero());
}

void SpinBasedTool::Attach() {
    auto model = GetModelAttachedTo();
    ASSERT(model);
    const auto model_size = MatchOperandModelAndGetSize(false);

    // Get the Spin from the attached Model in stage coordinates.
    stage_spin_ = GetStageSpinFromModel_();

    // Compute the radius and the size of the model along the spin axis.
    radius_ = .5f * ion::math::Length(model_size);
    size_along_axis_ =
        GetBoundsSizeAlongVector(model_size, stage_spin_.axis);

    // Update the widget size based on the size of the ConvertedModel.
    spin_widget_->SetSize(radius_, size_along_axis_);

    UpdateSpinWidget_();
}

void SpinBasedTool::Detach() {
}

void SpinBasedTool::Activate_(bool is_activation) {
    const auto &context = GetContext();

    stage_spin_ = GetStageSpinFromWidget_();

    if (is_activation) {
        start_stage_spin_ = stage_spin_;
        context.target_manager->StartSnapping();
    }
    else {
        spin_widget_->UnhighlightSubWidget("Rotator");
        spin_widget_->UnhighlightSubWidget("Translator");
        spin_widget_->UnhighlightSubWidget("Ring");
        context.target_manager->EndSnapping();
        if (angle_feedback_) {
            context.feedback_manager->Deactivate(angle_feedback_);
            angle_feedback_.reset();
        }
        if (offset_feedback_) {
            context.feedback_manager->Deactivate(offset_feedback_);
            offset_feedback_.reset();
        }

        GetDragEnded().Notify(*this);

        // If there was a significant enough change due to a drag, execute the
        // command to change the Model(s).
        if (command_) {
            const Spin &new_spin = command_->GetSpin();
            if (SpinsDiffer_(start_stage_spin_, new_spin))
                GetContext().command_manager->AddAndDo(command_);
            command_.reset();
        }
    }
}

void SpinBasedTool::SpinChanged_(SpinWidget::ChangeType type) {
    const auto &context = GetContext();

    stage_spin_ = GetStageSpinFromWidget_();

    // If this is the first change, create the ChangeSpinCommand and start the
    // drag.
    if (! command_) {
        command_ = CreateChangeSpinCommand();
        command_->SetFromSelection(GetSelection());
        GetDragStarted().Notify(*this);
    }

    // Undo any highlighting to start.
    spin_widget_->UnhighlightSubWidget("Rotator");
    spin_widget_->UnhighlightSubWidget("Translator");

    stage_spin_ = GetStageSpinFromWidget_();

    // Try snapping unless modified dragging.
    const bool is_snapped = ! context.is_modified_mode &&
        (type == SpinWidget::ChangeType::kAxis   ? SnapAxis_() :
         type == SpinWidget::ChangeType::kCenter ? SnapCenter_() : false);

    if (is_snapped) {
        // If snapped, update the SpinWidget to reflect the change.
        UpdateSpinWidget_();
    }
    else {
        // If not snapped, apply precision to the angle and offset unless
        // modified dragging.
        if (! context.is_modified_mode) {
            if (type == SpinWidget::ChangeType::kAngle)
                stage_spin_.angle =
                    context.precision_store->ApplyAngle(stage_spin_.angle);
            else if (type == SpinWidget::ChangeType::kOffset)
                stage_spin_.offset =
                    context.precision_store->Apply(stage_spin_.offset);
        }
    }

    command_->SetSpin(stage_spin_);
    context.command_manager->SimulateDo(command_);

    // Update angle or offset feedback.
    if (type == SpinWidget::ChangeType::kAngle) {
        if (! angle_feedback_)
            angle_feedback_ =
                context.feedback_manager->Activate<AngularFeedback>();
        UpdateAngleFeedback_();
    }
    else if (type == SpinWidget::ChangeType::kOffset) {
        if (! offset_feedback_)
            offset_feedback_ =
                context.feedback_manager->Activate<LinearFeedback>();
        UpdateOffsetFeedback_();
    }
}

bool SpinBasedTool::SnapAxis_() {
    auto &tm = *GetContext().target_manager;
    bool is_snapped = false;

    // Try to snap to the point target direction (in stage coordinates) if it
    // is active.  Otherwise, try to snap to any of the principal axes.
    Rotationf rot;
    if (tm.SnapToDirection(stage_spin_.axis, rot)) {
        stage_spin_.axis = tm.GetPointTarget().GetDirection();
        spin_widget_->HighlightSubWidget("Rotator", GetSnappedFeedbackColor());
        is_snapped = true;
    }
    else {
        const int snapped_dim = SnapToAxis(stage_spin_.axis);
        if (snapped_dim >= 0) {
            spin_widget_->HighlightSubWidget(
                "Rotator", SG::ColorMap::SGetColorForDimension(snapped_dim));
            is_snapped = true;
        }
    }

    return is_snapped;
}

bool SpinBasedTool::SnapCenter_() {
    auto &tm = *GetContext().target_manager;

    // Snapping is done in stage coordinates.  Try to snap to the point target
    // position (if it is active).  Otherwise, try to snap to any of the
    // principal points of the operand Model's bounds.
    bool is_snapped = false;
    Vector3f motion = stage_spin_.center - start_stage_spin_.center;
    if (tm.SnapToPoint(start_stage_spin_.center, motion)) {
        stage_spin_.center = start_stage_spin_.center + motion;
        is_snapped = true;
    }
    else {
        ConvertedModelPtr cm =
            std::dynamic_pointer_cast<ConvertedModel>(GetModelAttachedTo());
        ASSERT(cm);
        // Convert the operand Model's bounds into stage coordinates, which
        // requires undoing the offset translation from the ConvertedModel.
        const auto bounds =
            TranslateBounds(TransformBounds(
                                cm->GetOperandModel()->GetScaledBounds(),
                                GetStageCoordConv().GetObjectToRootMatrix()),
                            -cm->GetObjectCenterOffset());
        Point3f center = stage_spin_.center;
        const Dimensionality snapped_dims = Snap3D::SnapToBounds(
            bounds, center, TK::kSnapPointTolerance * Vector3f(1, 1, 1));
        if (snapped_dims.GetCount() > 1) {
            stage_spin_.center = center;
            is_snapped = true;
        }
    }

    if (is_snapped)
        spin_widget_->HighlightSubWidget(
            "Translator", GetSnappedFeedbackColor());

    return is_snapped;
}

Spin SpinBasedTool::GetStageSpinFromModel_() const {
    // Let the derived class get the Spin in object coordinates from the
    // Model, convert it to stage coordinates, and then undo the centering
    // translation.
    Spin spin = TransformSpin(GetObjectSpinFromModel(),
                              GetStageCoordConv().GetObjectToRootMatrix());
    spin.center -= GetModelAttachedTo()->GetLocalCenterOffset();
    return spin;
}

Spin SpinBasedTool::GetStageSpinFromWidget_() const {
    // Need to apply the current rotation and translation of the tool to the
    // SpinWidget's Spin. Since a SpinBasedTool is never scaled, its model
    // matrix should do the trick.
    return TransformSpin(spin_widget_->GetSpin(), GetModelMatrix());
}

void SpinBasedTool::UpdateSpinWidget_() {
    // This is the reverse of GetStageSpinFromWidget_().
    spin_widget_->SetSpin(
        TransformSpin(stage_spin_, ion::math::Inverse(GetModelMatrix())));
}

void SpinBasedTool::UpdateAngleFeedback_() {
    // The feedback should be in the plane perpendicular to the Spin axis (in
    // stage coordinates).
    const Point3f center =
        stage_spin_.center + size_along_axis_ * stage_spin_.axis;
    ASSERT(angle_feedback_);
    angle_feedback_->SubtendArc(center, 0, 0, stage_spin_.axis,
                                CircleArc(Anglef(), stage_spin_.angle));
}

void SpinBasedTool::UpdateOffsetFeedback_() {
    // The feedback ranges from the starting position of the offset slider to
    // its current position. Use SpanLength() instead of SpanPoints() here
    // because the offset may be 0. The slider starts at the angle=0 position
    // on the ring.
    const auto rot = Rotationf::FromAxisAndAngle(stage_spin_.axis,
                                                 stage_spin_.angle);
    const Point3f p0 = rot * (Point3f::Zero() + radius_ * Vector3f::AxisX());
    const float length = stage_spin_.offset;
    ASSERT(offset_feedback_);
    offset_feedback_->SpanLength(p0, stage_spin_.axis, length);
}

bool SpinBasedTool::SpinsDiffer_(const Spin &spin0, const Spin &spin1) {
    const Anglef angle_tolerance = Anglef::FromDegrees(.01f);
    return
        !           AreClose(spin0.center, spin1.center)                 ||
        ! AreDirectionsClose(spin0.axis,   spin1.axis, angle_tolerance)  ||
        !           AreClose(spin0.angle,  spin1.angle, angle_tolerance) ||
        !           AreClose(spin0.offset, spin1.offset);
}

float SpinBasedTool::GetBoundsSizeAlongVector(const Vector3f &bounds_size,
                                              const Vector3f &dir) {
    Point3f corners[8];
    Bounds(bounds_size).GetCorners(corners);
    float min_dist =  std::numeric_limits<float>::max();
    float max_dist = -std::numeric_limits<float>::max();
    for (const auto &corner: corners) {
        const float dist = SignedDistance(corner, dir);
        min_dist = std::min(min_dist, dist);
        max_dist = std::max(max_dist, dist);
    }
    return max_dist - min_dist;
}
