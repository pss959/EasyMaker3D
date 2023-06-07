#include "Tools/PlaneBasedTool.h"

#include <algorithm>
#include <limits>

#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Commands/ChangePlaneCommand.h"
#include "Feedback/LinearFeedback.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/TargetManager.h"
#include "Math/Linear.h"
#include "Models/Model.h"
#include "Place/PointTarget.h"
#include "SG/ColorMap.h"
#include "SG/CoordConv.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/Tuning.h"
#include "Widgets/PlaneWidget.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/SphereWidget.h"

PlaneBasedTool::PlaneBasedTool() {
}

void PlaneBasedTool::CreationDone() {
    Tool::CreationDone();

    if (! IsTemplate()) {
        plane_widget_ =
            SG::FindTypedNodeUnderNode<PlaneWidget>(*this, "PlaneWidget");

        // Set up callbacks.
        plane_widget_->GetActivation().AddObserver(
            this, [&](Widget &, bool is_act){ Activate_(is_act); });
        plane_widget_->GetPlaneChanged().AddObserver(
            this, [&](bool is_rotation){ PlaneChanged_(is_rotation); });
    }
}

void PlaneBasedTool::UpdateGripInfo(GripInfo &info) {
    // If the direction is close to the plane normal (in either direction), use
    // the translator.
    const Vector3f &guide_dir = info.guide_direction;
    const Vector3f &normal    = plane_widget_->GetPlane().normal;
    WidgetPtr widget;
    if (AreDirectionsClose(guide_dir,  normal, TK::kMaxGripHoverDirAngle) ||
        AreDirectionsClose(guide_dir, -normal, TK::kMaxGripHoverDirAngle)) {
        widget          = plane_widget_->GetSubWidget("PlaneTranslator");
        info.guide_type = GripGuideType::kBasic;
    }
    else {
        widget          = plane_widget_->GetSubWidget("AxisWidget");
        info.guide_type = GripGuideType::kRotation;
    }
    info.widget = Util::CastToDerived<ClickableWidget>(widget);
    ASSERT(info.widget);
    info.target_point = ToWorld(info.widget, Point3f::Zero());
}

void PlaneBasedTool::Attach() {
    auto model = GetModelAttachedTo();
    ASSERT(model);

    // Update the widget size based on the size of the operand Model.
    const auto model_size = MatchOperandModelAndGetSize(true);
    const float radius = .5f * ion::math::Length(model_size);
    plane_widget_->SetSize(radius);

    // Get the Plane from the attached Model in stage coordinates.
    stage_plane_ = GetStagePlaneFromModel_();

    // Update the PlaneWidget from the stage_plane_.
    UpdatePlaneWidgetPlane_();

    // Let the derived class impose a range on translation if necessary.
    UpdateTranslationRange_();
}

void PlaneBasedTool::Detach() {
}

Plane PlaneBasedTool::GetObjectPlane() const {
    // Use the distance from the PlaneWidget's plane, since that is based on
    // the actual plane translation.
    return Plane(plane_widget_->GetPlane().distance,
                 TransformNormal(stage_plane_.normal,
                                 GetStageCoordConv().GetRootToObjectMatrix()));
}

void PlaneBasedTool::Activate_(bool is_activation) {
    const auto &context = GetContext();

    stage_plane_ = GetStagePlaneFromWidget_();

    if (is_activation) {
        // Save the center of the Model in stage coordinates.
        stage_center_ = Point3f(GetTranslation());
        start_stage_plane_ = stage_plane_;
        context.target_manager->StartSnapping();
    }
    else {
        plane_widget_->UnhighlightSubWidget("Rotator");
        plane_widget_->UnhighlightSubWidget("Translator");
        context.target_manager->EndSnapping();
        if (feedback_) {
            context.feedback_manager->Deactivate(feedback_);
            feedback_.reset();
        }

        GetDragEnded().Notify(*this);
        UpdateTranslationRange_();

        // If there was a significant enough change due to a drag, execute the
        // command to change the Model(s).
        if (command_) {
            const Plane &new_plane = command_->GetPlane();
            if (! AreClose(new_plane.distance, start_stage_plane_.distance) ||
                ! AreDirectionsClose(new_plane.normal,
                                     start_stage_plane_.normal,
                                     Anglef::FromDegrees(.01f)))
                GetContext().command_manager->AddAndDo(command_);
            command_.reset();
        }
    }
}

void PlaneBasedTool::PlaneChanged_(bool is_rotation) {
    const auto &context = GetContext();

    stage_plane_ = GetStagePlaneFromWidget_();

    // If this is the first change, create the ChangePlaneCommand and start the
    // drag.
    if (! command_) {
        command_ = CreateChangePlaneCommand();
        command_->SetFromSelection(GetSelection());
        GetDragStarted().Notify(*this);
    }

    // Undo any highlighting to start.
    plane_widget_->UnhighlightSubWidget("Rotator");
    plane_widget_->UnhighlightSubWidget("Translator");

    // Try snapping unless modified dragging.
    const bool is_snapped = ! context.is_modified_mode &&
        (is_rotation ? SnapRotation_() : SnapTranslation_());

    command_->SetPlane(stage_plane_);
    context.command_manager->SimulateDo(command_);

    // Update translation feedback.
    if (! is_rotation) {
        if (! feedback_)
            feedback_ = context.feedback_manager->Activate<LinearFeedback>();
        UpdateTranslationFeedback_(is_snapped);
    }
}

bool PlaneBasedTool::SnapRotation_() {
    auto &tm = *GetContext().target_manager;
    bool is_snapped = false;

    // Try to snap to the point target direction (in stage coordinates) if it
    // is active.  Otherwise, try to snap to any of the principal axes.
    Rotationf rot;
    if (tm.SnapToDirection(stage_plane_.normal, rot)) {
        stage_plane_.normal = tm.GetPointTarget().GetDirection();
        plane_widget_->HighlightSubWidget("Rotator", GetSnappedFeedbackColor());
        is_snapped = true;
    }
    else {
        const int snapped_dim = SnapToAxis(stage_plane_.normal);
        if (snapped_dim >= 0) {
            plane_widget_->HighlightSubWidget(
                "Rotator", SG::ColorMap::SGetColorForDimension(snapped_dim));
            is_snapped = true;
        }
    }

    if (is_snapped) {
        // Maintain the same distance from the center.
        ASSERT(AreClose(ion::math::Length(stage_plane_.normal), 1));
        const float dist = start_stage_plane_.GetDistanceToPoint(stage_center_);
        const Point3f plane_pt = stage_center_ - dist * stage_plane_.normal;
        stage_plane_ = Plane(plane_pt, stage_plane_.normal);
        UpdatePlaneWidgetPlane_();
    }
    return is_snapped;
}

bool PlaneBasedTool::SnapTranslation_() {
    auto &tm = *GetContext().target_manager;

    // Try to snap to the point target position (if it is active) or the center
    // of the Model, whichever is closer.
    float dist = stage_plane_.GetDistanceToPoint(stage_center_);
    if (tm.IsPointTargetVisible()) {
        const float target_dist =
            stage_plane_.GetDistanceToPoint(tm.GetPointTarget().GetPosition());
        if (std::abs(target_dist) < std::abs(dist))
            dist = target_dist;
    }
    const bool is_snapped = std::abs(dist) <= TK::kSnapPointTolerance;
    if (is_snapped) {
        stage_plane_.distance += dist;
        UpdatePlaneWidgetPlane_();
        plane_widget_->HighlightSubWidget("Translator",
                                          GetSnappedFeedbackColor());
    }
    return is_snapped;
}

Plane PlaneBasedTool::GetStagePlaneFromModel_() const {
    // Let the derived class get the Plane in object coordinates from the
    // Model, convert it to stage coordinates, and then undo the centering
    // translation.
    return TranslatePlane(
        TransformPlane(GetObjectPlaneFromModel(),
                       GetStageCoordConv().GetObjectToRootMatrix()),
        -GetModelAttachedTo()->GetLocalCenterOffset());
}

Plane PlaneBasedTool::GetStagePlaneFromWidget_() const {
    // Need to apply the current rotation and translation of the tool to the
    // PlaneWidget's Plane. Since a PlaneBasedTool is never scaled, its model
    // matrix should do the trick.
    return TransformPlane(plane_widget_->GetPlane(), GetModelMatrix());
}

void PlaneBasedTool::UpdatePlaneWidgetPlane_() {
    // This is the reverse of GetStagePlaneFromWidget_().
    plane_widget_->SetPlane(
        TransformPlane(stage_plane_, ion::math::Inverse(GetModelMatrix())));
}

void PlaneBasedTool::UpdateTranslationRange_() {
    plane_widget_->SetTranslationRange(GetTranslationRange());
}

void PlaneBasedTool::UpdateTranslationFeedback_(bool is_snapped) {
    // Show the motion in the direction of the plane normal. Find the position
    // of the minimum point (relative to the plane) in stage coordinates.
    // XXXX Do something better?
    const auto &model = *GetModelAttachedTo();
    const auto &mesh        = model.GetMesh();
    const Vector3f &scale   = model.GetScale();
    float min_dist = std::numeric_limits<float>::max();
    const auto object_plane = GetObjectPlane();
    Point3f min_pt(0, 0, 0);
    for (const Point3f &p: mesh.points) {
        const Point3f scaled_pt = ScalePoint(p, scale);
        const float dist = SignedDistance(scaled_pt, object_plane.normal);
        if (dist < min_dist) {
            min_dist = dist;
            min_pt   = scaled_pt;
        }
    }
    const Point3f p0 = GetModelMatrix() * min_pt + model.GetLocalCenterOffset();
    const Point3f p1 = Point3f(stage_plane_.distance * stage_plane_.normal);
    feedback_->SetColor(is_snapped ? GetSnappedFeedbackColor() :
                        Color::White());
    feedback_->SpanLength(p0, stage_plane_.normal,
                          ion::math::Dot(p1 - p0, stage_plane_.normal));
}
