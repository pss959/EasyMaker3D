#include "Tools/TwistTool.h"

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Commands/ChangeTwistCommand.h"
#include "Feedback/AngularFeedback.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/TargetManager.h"
#include "Math/Curves.h"
#include "Models/TwistedModel.h"
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

void TwistTool::CreationDone() {
    Tool::CreationDone();

    if (! IsTemplate())
        SetUpParts_();
}

void TwistTool::UpdateGripInfo(GripInfo &info) {
    info.guide_type = GripGuideType::kRotation;

    // If the direction is close to the axis (in either direction), use the
    // twister.
    const Vector3f &guide_dir = info.guide_direction;
    if (AreDirectionsClose(guide_dir,  twist_.axis,
                           TK::kMaxGripHoverDirAngle) ||
        AreDirectionsClose(guide_dir, -twist_.axis,
                           TK::kMaxGripHoverDirAngle)) {
        info.widget       = twister_;
        info.target_point = ToWorld(info.widget, Point3f::Zero());
    }
    else {
        // Otherwise, use the axis rotator.
        info.widget = rotator_;
        // Connect to the min/max handle, whichever is higher up.
        const Point3f min_pt = ToWorld(base_, Point3f::Zero());
        const Point3f max_pt = ToWorld(cone_, Point3f::Zero());
        info.target_point = min_pt[1] > max_pt[1] ? min_pt : max_pt;
    }
}

bool TwistTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<TwistedModel>(sel);
}

void TwistTool::Attach() {
    UpdateGeometry_();

    // Match the Twist in the primary TwistedModel without notifying.
    const auto tm = Util::CastToDerived<TwistedModel>(GetModelAttachedTo());
    ASSERT(tm);
    twist_ = tm->GetTwist();
    MatchCurrentTwist_();
}

void TwistTool::SetUpParts_() {
    // Find all of the parts.
    rotator_      = SG::FindTypedNodeUnderNode<SphereWidget>(*this, "Rotator");
    twister_      = SG::FindTypedNodeUnderNode<DiscWidget>(*this,   "Twister");
    translator_   = SG::FindTypedNodeUnderNode<Slider2DWidget>(*this,
                                                               "Translator");
    axis_rotator_ = SG::FindNodeUnderNode(*this, "AxisRotator");
    axis_         = SG::FindNodeUnderNode(*translator_, "Axis");
    cone_         = SG::FindNodeUnderNode(*rotator_, "Cone");
    base_         = SG::FindNodeUnderNode(*rotator_, "Base");

    // Set a wide range for the translator.
    translator_->SetRange(Vector2f(-100, -100), Vector2f(100, 100));

    // Set up callbacks.
    twister_->GetActivation().AddObserver(
        this, [&](Widget &w, bool is_act){ Activate_(w, is_act); });
    twister_->GetRotationChanged().AddObserver(
        this, [&](Widget &w, const Anglef &){ TwistChanged_(w); });
    rotator_->GetActivation().AddObserver(
        this, [&](Widget &w, bool is_act){ Activate_(w, is_act); });
    rotator_->GetRotationChanged().AddObserver(
        this, [&](Widget &w, const Rotationf &){ TwistChanged_(w); });
    translator_->GetActivation().AddObserver(
        this, [&](Widget &w, bool is_act){ Activate_(w, is_act); });
    translator_->GetValueChanged().AddObserver(
        this, [&](Widget &w, const Vector2f &){ TwistChanged_(w); });
}

void TwistTool::UpdateGeometry_() {
    static const float kRadiusScale = .75f;
    static const float kAxisScale   = 1.2f;

    const Vector3f model_size = MatchOperandModelAndGetSize(false);
    const float radius = kRadiusScale * ion::math::Length(model_size);

    // Translate the axis rotator handles.
    const Vector3f y_trans(0, kAxisScale * radius, 0);
    base_->SetTranslation(-y_trans);
    cone_->SetTranslation(y_trans);

    // Scale the center translator axis height.
    const auto axis = SG::FindNodeUnderNode(*translator_, "Axis");
    auto scale = axis_->GetScale();
    scale[1] = 2 * kAxisScale * radius;
    axis_->SetScale(scale);

    // Scale and translate the twister parts.
    for (const auto &dim_char: std::string("XZ")) {
        const std::string dim_str(1, dim_char);
        const auto min   = SG::FindNodeUnderNode(*twister_, dim_str + "Min");
        const auto max   = SG::FindNodeUnderNode(*twister_, dim_str + "Max");
        const auto stick = SG::FindNodeUnderNode(*twister_, dim_str + "Stick");
        auto yscale = stick->GetScale();
        yscale[0] = 2 * radius;
        stick->SetScale(yscale);
        const Vector3f xtrans(radius, 0, 0);
        min->SetTranslation(-xtrans);
        max->SetTranslation(xtrans);
    }
}

void TwistTool::MatchCurrentTwist_() {
    // Don't notify for widget changes.
    twister_->GetRotationChanged().EnableObserver(this, false);
    rotator_->GetRotationChanged().EnableObserver(this, false);
    translator_->GetValueChanged().EnableObserver(this, false);

    const Rotationf rot = Rotationf::RotateInto(Twist().axis, twist_.axis);
    rotator_->SetRotation(rot);
    rotator_->SetTranslation(rot * twist_.center);
    axis_rotator_->SetRotation(rot);
    translator_->SetValue(Vector2f(twist_.center[0], twist_.center[2]));
    twister_->SetRotationAngle(twist_.angle);

    twister_->GetRotationChanged().EnableObserver(this, true);
    rotator_->GetRotationChanged().EnableObserver(this, true);
    translator_->GetValueChanged().EnableObserver(this, true);
}

void TwistTool::Detach() {
    // Nothing to do here.
}

void TwistTool::Activate_(Widget &widget, bool is_activation) {
    const auto &context = GetContext();
    if (is_activation) {
        const auto tm = Util::CastToDerived<TwistedModel>(GetModelAttachedTo());
        ASSERT(tm);
        start_twist_ = twist_ = tm->GetTwist();
        if (&widget == twister_.get()) {
            feedback_ = context.feedback_manager->Activate<AngularFeedback>();
            UpdateTwistFeedback_();
        }
        context.target_manager->StartSnapping();
    }
    else {
        context.target_manager->EndSnapping();
        if (&widget == twister_.get()) {
            context.feedback_manager->Deactivate(feedback_);
            feedback_.reset();
        }

        GetDragEnded().Notify(*this);

        // Execute the command to change the TwistedModel(s).
        if (command_) {
            if (command_->GetTwist() != start_twist_)
                GetContext().command_manager->AddAndDo(command_);
            command_.reset();
        }
    }
}

void TwistTool::TwistChanged_(Widget &widget) {
    // If this is the first change, create the ChangeTwistCommand and start the
    // drag.
    if (! command_) {
        command_ = CreateCommand<ChangeTwistCommand>();
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

    // If not snapped, update the current Twist from the active Widget's values
    // and match it.
    if (! is_snapped) {
        if (&widget == translator_.get()) {
            twist_.center = Twist().center + translator_->GetTranslation();
        }
        else if (&widget == rotator_.get()) {
            twist_.axis = rotator_->GetRotation() * Twist().axis;
        }
        else {
            // Apply precision to the angle unless modified dragging.
            const Anglef angle = twister_->GetRotationAngle();
            twist_.angle = ! context.is_modified_mode ?
                context.precision_store->ApplyAngle(angle) : angle;
        }
    }
    MatchCurrentTwist_();

    // Update the command.
    if (command_->GetTwist() != twist_) {
        command_->SetTwist(twist_);
        context.command_manager->SimulateDo(command_);
    }

    // Update feedback if twisting.
    if (&widget == twister_.get())
        UpdateTwistFeedback_();
}

bool TwistTool::SnapTranslation_() {
    using ion::math::Distance;

    auto &tm = *GetContext().target_manager;

    // Current center in object coordinates (from the Slider2DWidget).
    const auto cur_obj_pos = Twist().center + translator_->GetTranslation();

    // Try to snap to the point target position (if it is active) or the center
    // of the untwisted Model, whichever is closer. Work in stage coordinates.
    // Note that the conversion is from local coordinates since the translation
    // is after the scale.
    const auto stage_cc  = GetStageCoordConv();
    const auto lsm       = stage_cc.GetLocalToRootMatrix();
    const auto start_pos = lsm * start_twist_.center;
    const auto cur_pos   = lsm * cur_obj_pos;
    const auto model_pos = lsm * Point3f::Zero();

    bool is_snapped = false;

    Vector3f motion = cur_pos - start_pos;
    if (tm.SnapToPoint(start_pos, motion)) {
        twist_.center = stage_cc.RootToObject(start_pos + motion);
        is_snapped = true;
    }
    // Use twice the tolerance here because of 2 dimensions. :-)
    else if (Distance(cur_pos, model_pos) <= 2 * TK::kSnapPointTolerance) {
        twist_.center = Point3f::Zero();
        is_snapped = true;
    }
    if (is_snapped)
        translator_->SetActiveColor(GetSnappedFeedbackColor());

    return is_snapped;
}

bool TwistTool::SnapRotation_() {
    auto &tm = *GetContext().target_manager;

    bool is_snapped = false;

    // Try to snap to the point target direction (in stage coordinates) if it
    // is active.  Otherwise, try to snap to any of the principal axes.
    Vector3f axis = rotator_->GetRotation() * Twist().axis;
    const auto stage_cc = GetStageCoordConv();
    Vector3f stage_axis = stage_cc.ObjectToRoot(axis);
    Rotationf rot;
    rotator_->SetActiveColor(SG::ColorMap::SGetColor("WidgetActiveColor"));
    if (tm.SnapToDirection(stage_axis, rot)) {
        twist_.axis = stage_cc.RootToObject(tm.GetPointTarget().GetDirection());
        rotator_->SetActiveColor(GetSnappedFeedbackColor());
        is_snapped = true;
    }
    else {
        const int snapped_dim = SnapToAxis(stage_axis);
        is_snapped = snapped_dim >= 0;
        if (is_snapped) {
            twist_.axis = stage_cc.RootToObject(stage_axis);
            rotator_->SetActiveColor(
                SG::ColorMap::SGetColorForDimension(snapped_dim));
        }
    }
    // Make sure the axis is unit length.
    ion::math::Normalize(&twist_.axis);

    return is_snapped;
}

void TwistTool::UpdateTwistFeedback_() {
    // The feedback should be in the plane perpendicular to the twist axis (in
    // stage coordinates).
    const Vector3f stage_axis = GetRotation() * twist_.axis;

    // Move the feedback center to be just past the axis cone in stage
    // coordinates.
    const Point3f stage_center = Point3f(GetTranslation()) +
        (1.2f * cone_->GetTranslation()[1] * stage_axis);

    ASSERT(feedback_);
    feedback_->SubtendArc(stage_center, 0, 0, stage_axis,
                          CircleArc(Anglef(), twist_.angle));
}
