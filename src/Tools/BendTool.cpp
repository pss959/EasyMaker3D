#include "Tools/BendTool.h"

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Commands/ChangeBendCommand.h"
#include "Feedback/AngularFeedback.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/TargetManager.h"
#include "Math/Curves.h"
#include "Models/BentModel.h"
#include "Place/PointTarget.h"
#include "Place/PrecisionStore.h"
#include "Place/Snapping.h"
#include "SG/ColorMap.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/Tuning.h"
#include "Widgets/AxisWidget.h"
#include "Widgets/WheelWidget.h"
#include "Widgets/Slider2DWidget.h"
#include "Widgets/SphereWidget.h"

// XXXX Add a way to offset each 360 degree bend.

// XXXX Merge most of this with TwistTool?

void BendTool::CreationDone() {
    Tool::CreationDone();

    if (! IsTemplate())
        SetUpParts_();
}

void BendTool::UpdateGripInfo(GripInfo &info) {
    info.guide_type = GripGuideType::kRotation;

    // If the direction is close to the axis (in either direction), use the
    // bender.
    const Vector3f &guide_dir = info.guide_direction;
    if (AreDirectionsClose(guide_dir,  bend_.axis,
                           TK::kMaxGripHoverDirAngle) ||
        AreDirectionsClose(guide_dir, -bend_.axis,
                           TK::kMaxGripHoverDirAngle)) {
        info.widget = Util::CastToDerived<ClickableWidget>(
            bender_->GetSubWidget("Rotator"));
        info.target_point = ToWorld(info.widget, Point3f::Zero());
    }
    else {
#if XXXX
        // Otherwise, use the axis rotator.
        info.widget = rotator_;
        // Connect to the min/max handle, whichever is higher up.
        const Point3f min_pt = ToWorld(cone0_, Point3f::Zero());
        const Point3f max_pt = ToWorld(cone1_, Point3f::Zero());
        info.target_point = min_pt[1] > max_pt[1] ? min_pt : max_pt;
#endif
    }
}

bool BendTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<BentModel>(sel);
}

void BendTool::Attach() {
    UpdateGeometry_();

    // Match the Bend in the primary BentModel without notifying.
    const auto bm = Util::CastToDerived<BentModel>(GetModelAttachedTo());
    ASSERT(bm);
    bend_ = bm->GetBend();
    MatchCurrentBend_();
}

void BendTool::SetUpParts_() {
    // Find all of the parts.
    axis_   = SG::FindTypedNodeUnderNode<AxisWidget>(*this,   "AxisWidget");
    bender_ = SG::FindTypedNodeUnderNode<WheelWidget>(*this, "Wheel");

    // Set a wide range for the axis translation.
    axis_->SetTranslationRange(Range2f(Point2f(-100, -100), Point2f(100, 100)));

#if XXXX
    translator_   = SG::FindTypedNodeUnderNode<Slider2DWidget>(*this,
                                                               "Translator");
    rotator_      = SG::FindTypedNodeUnderNode<SphereWidget>(*this, "Rotator");
    axis_rotator_ = SG::FindNodeUnderNode(*this, "AxisRotator");
    axis_         = SG::FindNodeUnderNode(*translator_, "Axis");
    cone0_        = SG::FindNodeUnderNode(*rotator_, "Cone0");
    cone1_        = SG::FindNodeUnderNode(*rotator_, "Cone1");

    // Set a wide range for the translator.
    translator_->SetRange(Vector2f(-100, -100), Vector2f(100, 100));
#endif

#if XXXX
    // Set up callbacks.
    bender_->GetActivation().AddObserver(
        this, [&](Widget &w, bool is_act){ Activate_(w, is_act); });
    bender_->GetRotationChanged().AddObserver(
        this, [&](Widget &w, const Anglef &){ BendChanged_(w); });
    rotator_->GetActivation().AddObserver(
        this, [&](Widget &w, bool is_act){ Activate_(w, is_act); });
    rotator_->GetRotationChanged().AddObserver(
        this, [&](Widget &w, const Rotationf &){ BendChanged_(w); });
    translator_->GetActivation().AddObserver(
        this, [&](Widget &w, bool is_act){ Activate_(w, is_act); });
    translator_->GetValueChanged().AddObserver(
        this, [&](Widget &w, const Vector2f &){ BendChanged_(w); });
#endif
}

void BendTool::UpdateGeometry_() {
    const Vector3f model_size = MatchOperandModelAndGetSize(false);
    const float radius = .5f * ion::math::Length(model_size);
    axis_->SetSize(radius);
    bender_->SetSize(radius);
}

void BendTool::MatchCurrentBend_() {
    const Rotationf rot = Rotationf::RotateInto(Bend().axis, bend_.axis);

    axis_->SetDirection(bend_.axis);
    axis_->SetPosition(rot * bend_.center);

    // Don't notify for widget changes.
    bender_->GetRotationChanged().EnableAll(false);
    bender_->SetRotation(rot);
    bender_->SetRotationAngle(bend_.angle);
    bender_->GetRotationChanged().EnableAll(true);
}

void BendTool::Detach() {
    // Nothing to do here.
}

void BendTool::Activate_(Widget &widget, bool is_activation) {
    const auto &context = GetContext();
    if (is_activation) {
        const auto bm = Util::CastToDerived<BentModel>(GetModelAttachedTo());
        ASSERT(bm);
        start_bend_ = bend_ = bm->GetBend();
        if (&widget == bender_.get()) {
            feedback_ = context.feedback_manager->Activate<AngularFeedback>();
            UpdateBendFeedback_();
        }
        context.target_manager->StartSnapping();
    }
    else {
        context.target_manager->EndSnapping();
        if (&widget == bender_.get()) {
            context.feedback_manager->Deactivate(feedback_);
            feedback_.reset();
        }

        GetDragEnded().Notify(*this);

        // Execute the command to change the BentModel(s).
        if (command_) {
            if (command_->GetBend() != start_bend_)
                GetContext().command_manager->AddAndDo(command_);
            command_.reset();
        }
    }
}

void BendTool::BendChanged_(Widget &widget) {
    // If this is the first change, create the ChangeBendCommand and start the
    // drag.
    if (! command_) {
        command_ = CreateCommand<ChangeBendCommand>();
        command_->SetFromSelection(GetSelection());
        GetDragStarted().Notify(*this);
    }

    const Color wac = SG::ColorMap::SGetColor("WidgetActiveColor");
    axis_->SetActiveColor(wac);

    // Try snapping if rotating or translating unless modified dragging.
    const auto &context = GetContext();
#if XXXX
    const bool is_snapped = ! context.is_modified_mode &&
        (&widget == translator_.get() ? SnapTranslation_() :
         &widget == rotator_.get()    ? SnapRotation_() : false);

    // If not snapped, update the current Bend from the active Widget's values
    // and match it.
    if (! is_snapped) {
        if (&widget == translator_.get()) {
            bend_.center = Bend().center + translator_->GetTranslation();
        }
        else if (&widget == rotator_.get()) {
            bend_.axis = rotator_->GetRotation() * Bend().axis;
        }
        else {
            // Apply precision to the angle unless modified dragging.
            const Anglef angle = bender_->GetRotationAngle();
            bend_.angle = ! context.is_modified_mode ?
                context.precision_store->ApplyAngle(angle) : angle;
        }
    }
#endif
    MatchCurrentBend_();

    // Update the command.
    if (command_->GetBend() != bend_) {
        command_->SetBend(bend_);
        context.command_manager->SimulateDo(command_);
    }

    // Update feedback if bending.
    if (&widget == bender_.get())
        UpdateBendFeedback_();
}

bool BendTool::SnapTranslation_() {
    using ion::math::Distance;

    auto &tm = *GetContext().target_manager;

    // Current center in object coordinates (from the AxisWidget).
    const auto cur_obj_pos = axis_->GetPosition();

    // Snap in stage coordinates.
    const auto stage_cc  = GetStageCoordConv();
    const auto osm       = stage_cc.GetObjectToRootMatrix();
    const auto start_pos = osm * start_bend_.center;
    const auto cur_pos   = osm * cur_obj_pos;

    bool is_snapped = false;

    // Try to snap to the point target position (if it is active) or any of the
    // principal points of the unbent Model's bounds, whichever is
    Vector3f motion = cur_pos - start_pos;
    if (tm.SnapToPoint(start_pos, motion)) {
        bend_.center = stage_cc.RootToObject(start_pos + motion);
        is_snapped = true;
    }
    // Otherwise, try to snap to any of the principal points of the unbent
    // Model's bounds.
    else {
        const Bounds stage_bounds = GetStageBounds();
        Point3f stage_pos = cur_pos;
        const Vector3f tolerance = TK::kSnapPointTolerance * Vector3f(1, 1, 1);
        const Dimensionality snapped_dims =
            SnapToBounds(stage_bounds, stage_pos, tolerance);
        if (snapped_dims.GetCount()) {
            bend_.center = stage_cc.RootToObject(stage_pos);
            is_snapped = true;
        }
    }
#if XXXX
    if (is_snapped)
        translator_->SetActiveColor(GetSnappedFeedbackColor());
#endif

    return is_snapped;
}

bool BendTool::SnapRotation_() {
    bool is_snapped = false;
#if XXXX
    auto &tm = *GetContext().target_manager;

    // Try to snap to the point target direction (in stage coordinates) if it
    // is active.  Otherwise, try to snap to any of the principal axes.
    Vector3f axis = rotator_->GetRotation() * Bend().axis;
    const auto stage_cc = GetStageCoordConv();
    Vector3f stage_axis = stage_cc.ObjectToRoot(axis);
    Rotationf rot;
    rotator_->SetActiveColor(SG::ColorMap::SGetColor("WidgetActiveColor"));
    if (tm.SnapToDirection(stage_axis, rot)) {
        bend_.axis = stage_cc.RootToObject(tm.GetPointTarget().GetDirection());
        rotator_->SetActiveColor(GetSnappedFeedbackColor());
        is_snapped = true;
    }
    else {
        const int snapped_dim = SnapToAxis(stage_axis);
        is_snapped = snapped_dim >= 0;
        if (is_snapped) {
            bend_.axis = stage_cc.RootToObject(stage_axis);
            rotator_->SetActiveColor(
                SG::ColorMap::SGetColorForDimension(snapped_dim));
        }
    }
    // Make sure the axis is unit length.
    ion::math::Normalize(&bend_.axis);
#endif

    return is_snapped;
}

void BendTool::UpdateBendFeedback_() {
#if XXXX
    // The feedback should be in the plane perpendicular to the bend axis (in
    // stage coordinates).
    const Vector3f stage_axis = GetRotation() * bend_.axis;

    // Move the feedback center to be just past the axis cone in stage
    // coordinates.
    const Point3f stage_center = Point3f(GetTranslation()) +
        (1.2f * cone1_->GetTranslation()[1] * stage_axis);

    ASSERT(feedback_);
    feedback_->SubtendArc(stage_center, 0, 0, stage_axis,
                          CircleArc(Anglef(), bend_.angle));
#endif
}
