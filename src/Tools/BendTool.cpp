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
#include "SG/ColorMap.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/Tuning.h"
#include "Widgets/DiscWidget.h"
#include "Widgets/Slider2DWidget.h"
#include "Widgets/SphereWidget.h"

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
        info.widget       = bender_;
        info.target_point = ToWorld(info.widget, Point3f::Zero());
    }
    else {
        // Otherwise, use the axis rotator.
        info.widget = rotator_;
        // Connect to the min/max handle, whichever is higher up.
        const Point3f min_pt = ToWorld(cone0_, Point3f::Zero());
        const Point3f max_pt = ToWorld(cone1_, Point3f::Zero());
        info.target_point = min_pt[1] > max_pt[1] ? min_pt : max_pt;
    }
}

bool BendTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<BentModel>(sel);
}

void BendTool::Attach() {
    UpdateGeometry_();

    // Match the Bend in the primary BentModel without notifying.
    const auto tm = Util::CastToDerived<BentModel>(GetModelAttachedTo());
    ASSERT(tm);
    bend_ = tm->GetBend();
    MatchCurrentBend_();
}

void BendTool::SetUpParts_() {
    // Find all of the parts.
    rotator_      = SG::FindTypedNodeUnderNode<SphereWidget>(*this, "Rotator");
    bender_      = SG::FindTypedNodeUnderNode<DiscWidget>(*this,   "Bender");
    translator_   = SG::FindTypedNodeUnderNode<Slider2DWidget>(*this,
                                                               "Translator");
    axis_rotator_ = SG::FindNodeUnderNode(*this, "AxisRotator");
    axis_         = SG::FindNodeUnderNode(*translator_, "Axis");
    cone0_        = SG::FindNodeUnderNode(*rotator_, "Cone0");
    cone1_        = SG::FindNodeUnderNode(*rotator_, "Cone1");

    // Set a wide range for the translator.
    translator_->SetRange(Vector2f(-100, -100), Vector2f(100, 100));

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
}

void BendTool::UpdateGeometry_() {
    static const float kRadiusScale = .75f;
    static const float kAxisScale   = 1.2f;

    // Rotate to match the Model. The BendTool always aligns with local axes.
    const Vector3f model_size = MatchModelAndGetSize(false);
    const float radius = kRadiusScale * ion::math::Length(model_size);

    // Translate the axis rotator handles.
    const Vector3f y_trans(0, kAxisScale * radius, 0);
    cone0_->SetTranslation(-y_trans);
    cone1_->SetTranslation(y_trans);

    // Scale the center translator axis height.
    const auto axis = SG::FindNodeUnderNode(*translator_, "Axis");
    auto scale = axis_->GetScale();
    scale[1] = 2 * kAxisScale * radius;
    axis_->SetScale(scale);

    // Scale and translate the bender parts.
    for (const auto &dim_char: std::string("XZ")) {
        const std::string dim_str(1, dim_char);
        const auto min   = SG::FindNodeUnderNode(*bender_, dim_str + "Min");
        const auto max   = SG::FindNodeUnderNode(*bender_, dim_str + "Max");
        const auto stick = SG::FindNodeUnderNode(*bender_, dim_str + "Stick");
        auto yscale = stick->GetScale();
        yscale[0] = 2 * radius;
        stick->SetScale(yscale);
        const Vector3f xtrans(radius, 0, 0);
        min->SetTranslation(-xtrans);
        max->SetTranslation(xtrans);
    }
}

void BendTool::MatchCurrentBend_() {
    // Don't notify for widget changes.
    bender_->GetRotationChanged().EnableObserver(this, false);
    rotator_->GetRotationChanged().EnableObserver(this, false);
    translator_->GetValueChanged().EnableObserver(this, false);

    // The tool points along +Y by default.
    const Rotationf rot = Rotationf::RotateInto(Bend().axis, bend_.axis);
    rotator_->SetRotation(rot);
    rotator_->SetTranslation(rot * bend_.center);
    axis_rotator_->SetRotation(rot);
    translator_->SetValue(Vector2f(bend_.center[0], bend_.center[2]));
    bender_->SetRotationAngle(bend_.angle);

    bender_->GetRotationChanged().EnableObserver(this, true);
    rotator_->GetRotationChanged().EnableObserver(this, true);
    translator_->GetValueChanged().EnableObserver(this, true);
}

void BendTool::Detach() {
    // Nothing to do here.
}

void BendTool::Activate_(Widget &widget, bool is_activation) {
    const auto &context = GetContext();
    if (is_activation) {
        const auto tm = Util::CastToDerived<BentModel>(GetModelAttachedTo());
        ASSERT(tm);
        start_bend_ = bend_ = tm->GetBend();
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

        // Execute the command to change the ClippedModel(s).
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
    rotator_->SetActiveColor(wac);
    translator_->SetActiveColor(wac);

    // Try snapping if rotating or translating unless modified dragging.
    const auto &context = GetContext();
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
    MatchCurrentBend_();

    // Update the command.
    command_->SetBend(bend_);
    context.command_manager->SimulateDo(command_);

    // Update feedback if bending.
    if (&widget == bender_.get())
        UpdateBendFeedback_();
}

bool BendTool::SnapTranslation_() {
    using ion::math::Distance;

    auto &tm = *GetContext().target_manager;

    // Current center in object coordinates (from the Slider2DWidget).
    const auto cur_obj_pos = Bend().center + translator_->GetTranslation();

    // Try to snap to the point target position (if it is active) or the center
    // of the unbent Model, whichever is closer. Work in stage coordinates.
    // Note that the conversion is from local coordinates since the translation
    // is after the scale.
    const auto stage_cc  = GetStageCoordConv();
    const auto lsm       = stage_cc.GetLocalToRootMatrix();
    const auto start_pos = lsm * start_bend_.center;
    const auto cur_pos   = lsm * cur_obj_pos;
    const auto model_pos = lsm * Point3f::Zero();

    bool is_snapped = false;

    Vector3f motion = cur_pos - start_pos;
    if (tm.SnapToPoint(start_pos, motion)) {
        bend_.center = stage_cc.RootToObject(start_pos + motion);
        is_snapped = true;
    }
    // Use twice the tolerance here because of 2 dimensions. :-)
    else if (Distance(cur_pos, model_pos) <= 2 * TK::kSnapPointTolerance) {
        bend_.center = Point3f::Zero();
        is_snapped = true;
    }
    if (is_snapped)
        translator_->SetActiveColor(GetSnappedFeedbackColor());

    return is_snapped;
}

bool BendTool::SnapRotation_() {
    auto &tm = *GetContext().target_manager;

    bool is_snapped = false;

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

    return is_snapped;
}

void BendTool::UpdateBendFeedback_() {
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
}
