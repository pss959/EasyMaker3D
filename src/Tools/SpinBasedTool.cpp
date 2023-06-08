#include "Tools/SpinBasedTool.h"

#include <algorithm>
#include <limits>

#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Commands/ChangeSpinCommand.h"
#include "Feedback/AngularFeedback.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/TargetManager.h"
#include "Math/Linear.h"
#include "Models/ConvertedModel.h"
#include "Place/PointTarget.h"
#include "Place/PrecisionStore.h"
#include "Place/Snapping.h"
#include "SG/ColorMap.h"
#include "SG/CoordConv.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/Tuning.h"
#include "Widgets/SpinWidget.h"

SpinBasedTool::SpinBasedTool() {
}

void SpinBasedTool::CreationDone() {
    Tool::CreationDone();

    if (! IsTemplate()) {
        spin_widget_ = SG::FindTypedNodeUnderNode<SpinWidget>(*this, "Spin");

        // Set up callbacks.
        spin_widget_->GetActivation().AddObserver(
            this, [&](Widget &, bool is_act){ Activate_(is_act); });
        spin_widget_->GetSpinChanged().AddObserver(
            this, [&](SpinWidget::ChangeType type){ SpinChanged_(type); });
    }
}

void SpinBasedTool::UpdateGripInfo(GripInfo &info) {
    // XXXX Figure this out!!!
#if XXXX
    // If the direction is close to the angle normal (in either direction), use
    // the translator.
    const Vector3f &guide_dir = info.guide_direction;
    const Vector3f &normal    = spin_widget_->GetSpin().normal;
    WidgetPtr widget;
    if (AreDirectionsClose(guide_dir,  normal, TK::kMaxGripHoverDirSpin) ||
        AreDirectionsClose(guide_dir, -normal, TK::kMaxGripHoverDirSpin)) {
        widget          = spin_widget_->GetSubWidget("Axis");
        info.guide_type = GripGuideType::kBasic;
    }
    else {
        widget          = spin_widget_->GetSubWidget("Ring");
        info.guide_type = GripGuideType::kRotation;
    }
    info.widget = Util::CastToDerived<ClickableWidget>(widget);
    ASSERT(info.widget);
    info.target_point = ToWorld(info.widget, Point3f::Zero());
#endif
}

void SpinBasedTool::Attach() {
    auto model = GetModelAttachedTo();
    ASSERT(model);

    // Update the widget size based on the size of the ConvertedModel.
    const auto model_size = MatchOperandModelAndGetSize(false);
    const float radius = .5f * ion::math::Length(model_size);
    spin_widget_->SetSize(radius);

    // Get the Spin from the attached Model in stage coordinates.
    stage_spin_ = GetStageSpinFromModel_();

    UpdateSpinWidget_();
}

void SpinBasedTool::Detach() {
}

Spin SpinBasedTool::GetObjectSpin() const {
    return TransformSpin(spin_widget_->GetSpin(),
                         GetStageCoordConv().GetRootToObjectMatrix());
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
        if (feedback_) {
            context.feedback_manager->Deactivate(feedback_);
            feedback_.reset();
        }

        GetDragEnded().Notify(*this);

        // If there was a significant enough change due to a drag, execute the
        // command to change the Model(s).
        if (command_) {
            const Spin &new_spin = command_->GetSpin();
            if (! AreClose(new_spin.center, start_stage_spin_.center) ||
                ! AreDirectionsClose(new_spin.axis, start_stage_spin_.axis,
                                     Anglef::FromDegrees(.01f)) ||
                ! AreClose(new_spin.angle, start_stage_spin_.angle,
                           Anglef::FromDegrees(.01f)))
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

    // Try snapping unless modified dragging.
    const bool is_snapped = ! context.is_modified_mode &&
        (type == SpinWidget::ChangeType::kAxis   ? SnapAxis_() :
         type == SpinWidget::ChangeType::kCenter ? SnapCenter_() : false);

    if (is_snapped) {
        // If snapped, update the SpinWidget to reflect the change.
        UpdateSpinWidget_();
    }
    else {
        // If not snapped, update the current Spin and match it.
        stage_spin_ = GetStageSpinFromWidget_();

        // Apply precision to the angle unless modified dragging.
        if (! context.is_modified_mode &&
            type == SpinWidget::ChangeType::kAngle) {
            stage_spin_.angle =
                context.precision_store->ApplyAngle(stage_spin_.angle);
        }
    }

    command_->SetSpin(stage_spin_);
    context.command_manager->SimulateDo(command_);

    // Update angle feedback.
    if (type == SpinWidget::ChangeType::kAngle) {
        if (! feedback_)
            feedback_ = context.feedback_manager->Activate<AngularFeedback>();
        UpdateAngleFeeedback_();
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
            Util::CastToDerived<ConvertedModel>(GetModelAttachedTo());
        ASSERT(cm);
        // Convert the operand Model's bounds into stage coordinates, which
        // requires undoing the offset translation from the ConvertedModel.
        const auto bounds =
            TranslateBounds(TransformBounds(
                                cm->GetOperandModel()->GetScaledBounds(),
                                GetStageCoordConv().GetObjectToRootMatrix()),
                            -cm->GetObjectCenterOffset());
        Point3f center = stage_spin_.center;
        const Dimensionality snapped_dims = SnapToBounds(
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

void SpinBasedTool::UpdateAngleFeeedback_() {
    // The feedback should be in the plane perpendicular to the Spin axis (in
    // stage coordinates).
    const Point3f center = stage_spin_.center + 1.2f * stage_spin_.axis;
    ASSERT(feedback_);
    feedback_->SubtendArc(center, 0, 0, stage_spin_.axis,
                          CircleArc(Anglef(), stage_spin_.angle));
}
