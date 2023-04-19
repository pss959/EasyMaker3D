#include "Tools/ClipTool.h"

#include <algorithm>
#include <limits>

#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Commands/ChangeClipCommand.h"
#include "Feedback/LinearFeedback.h"
#include "Items/SessionState.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/TargetManager.h"
#include "Math/Linear.h"
#include "Models/ClippedModel.h"
#include "Models/RootModel.h"
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

ClipTool::ClipTool() {
}

void ClipTool::CreationDone() {
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

void ClipTool::UpdateGripInfo(GripInfo &info) {
    // If the direction is close to the plane normal (in either direction), use
    // the translator.
    const Vector3f &guide_dir = info.guide_direction;
    const Vector3f &normal    = plane_widget_->GetPlane().normal;
    if (AreDirectionsClose(guide_dir,  normal, TK::kMaxGripHoverDirAngle) ||
        AreDirectionsClose(guide_dir, -normal, TK::kMaxGripHoverDirAngle)) {
        info.widget = plane_widget_->GetTranslator();
    }
    else {
        info.widget = plane_widget_->GetRotator();
    }
    info.target_point = ToWorld(info.widget, Point3f::Zero());
}

bool ClipTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<ClippedModel>(sel);
}

void ClipTool::Attach() {
    cm_ = Util::CastToDerived<ClippedModel>(GetModelAttachedTo());
    ASSERT(cm_);

    // Do not use MatchModelAndGetSize() here because the unclipped mesh has to
    // be used to get the correct size.

    // Rotate the ClipTool to align with the ClippedModel even if
    // is_axis_aligned is true.
    SetRotation(cm_->GetRotation());

    // Translate the ClipTool so that it is centered on the unclipped mesh.
    const Bounds &obj_bounds = cm_->GetOperandModel()->GetBounds();
    const Matrix4f osm = GetStageCoordConv().GetObjectToRootMatrix();
    SetTranslation(osm * obj_bounds.GetCenter() - cm_->GetLocalCenterOffset());

    // Update the widget size based on the size of the unclipped mesh.
    const auto model_size =
        ion::math::GetScaleVector(osm) * obj_bounds.GetSize();
    const float radius = .5f * ion::math::Length(model_size);
    plane_widget_->SetSize(radius);

    // Convert the ClippedModel's Plane from object to stage coordinates.
    stage_plane_ = GetStagePlaneFromModel_();

    // Update the PlaneWidget from the stage_plane_.
    UpdatePlaneWidgetPlane_();

    // Update the range of the slider based on the size of the Model and the
    // normal direction.
    UpdateTranslationRange_();
}

void ClipTool::Detach() {
    cm_.reset();
}

void ClipTool::Activate_(bool is_activation) {
    ASSERT(cm_);
    const auto &context = GetContext();

    stage_plane_ = GetStagePlaneFromWidget_();

    if (is_activation) {
        // Save the center of the unclipped Model in stage coordinates.
        stage_center_ = Point3f(GetTranslation());
        start_stage_plane_ = stage_plane_;
        feedback_ = context.feedback_manager->Activate<LinearFeedback>();
        context.target_manager->StartSnapping();
        UpdateRealTimeClipPlane_(true);
    }
    else {
        plane_widget_->UnhighlightArrowColor();
        context.target_manager->EndSnapping();
        context.feedback_manager->Deactivate(feedback_);
        feedback_.reset();

        GetDragEnded().Notify(*this);
        GetContext().target_manager->EndSnapping();

        UpdateTranslationRange_();

        // If there was a significant enough change due to a drag, execute the
        // command to change the ClippedModel(s).
        if (command_) {
            const Plane &new_plane = command_->GetPlane();
            if (! AreClose(new_plane.distance, start_stage_plane_.distance) ||
                ! AreDirectionsClose(new_plane.normal,
                                     start_stage_plane_.normal,
                                     Anglef::FromDegrees(.01f)))
                GetContext().command_manager->AddAndDo(command_);
            command_.reset();
        }
        UpdateRealTimeClipPlane_(false);
    }
}

void ClipTool::PlaneChanged_(bool is_rotation) {
    const auto &context = GetContext();

    stage_plane_ = GetStagePlaneFromWidget_();

    // If this is the first change, create the ChangeClipCommand and start the
    // drag.
    if (! command_) {
        command_ = CreateCommand<ChangeClipCommand>();
        command_->SetFromSelection(GetSelection());
        GetDragStarted().Notify(*this);
    }

    // Try snapping unless modified dragging.
    plane_widget_->UnhighlightArrowColor();
    Color color = Color::White();  // Changed if snapped.
    if (! context.is_modified_mode) {
        int snapped_dim = -1;
        const bool is_snapped = is_rotation ?
            SnapRotation_(snapped_dim) : SnapTranslation_();
        if (is_snapped) {
            color = snapped_dim >= 0 ?
                SG::ColorMap::SGetColorForDimension(snapped_dim) :
                GetSnappedFeedbackColor();
            plane_widget_->HighlightArrowColor(color);
        }
    }

    command_->SetPlane(stage_plane_);
    context.command_manager->SimulateDo(command_);

    UpdateRealTimeClipPlane_(true);

    // Update translation feedback.
    if (! is_rotation)
        UpdateTranslationFeedback_(color);
}

bool ClipTool::SnapRotation_(int &snapped_dim) {
    const auto &context = GetContext();
    auto &tm = *context.target_manager;

    bool is_snapped = false;
    snapped_dim     = -1;

    // Try to snap to the point target direction (in stage coordinates) if it
    // is active.
    Rotationf rot;
    if (tm.SnapToDirection(stage_plane_.normal, rot)) {
        stage_plane_.normal = tm.GetPointTarget().GetDirection();
        is_snapped = true;
    }

    // Otherwise, try to snap to any of the principal axes. If is_axis_aligned
    // is true, use the stage-coordinate axes as is. Otherwise, convert
    // object-coordinate axes into stage coordinates first.
    else {
        const bool use_stage_coords =
            context.command_manager->GetSessionState()->IsAxisAligned();
        const Matrix4f m = use_stage_coords ? Matrix4f::Identity() :
            GetStageCoordConv().GetObjectToRootMatrix();
        for (int dim = 0; dim < 3; ++dim) {
            const Vector3f axis = ion::math::Normalized(m * GetAxis(dim));
            if (tm.ShouldSnapDirections(stage_plane_.normal, axis, rot)) {
                stage_plane_.normal = axis;
                snapped_dim = dim;
                break;
            }
            else if (tm.ShouldSnapDirections(stage_plane_.normal, -axis, rot)) {
                stage_plane_.normal = -axis;
                snapped_dim = dim;
                break;
            }
        }
        is_snapped = snapped_dim >= 0;
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

bool ClipTool::SnapTranslation_() {
    auto &tm = *GetContext().target_manager;

    // Try to snap to the point target position (if it is active) or the center
    // of the unclipped Model, whichever is closer.
    float dist = stage_plane_.GetDistanceToPoint(stage_center_);
    if (tm.IsPointTargetVisible()) {
        const float target_dist =
            stage_plane_.GetDistanceToPoint(tm.GetPointTarget().GetPosition());
        if (std::abs(target_dist) < std::abs(dist))
            dist = target_dist;
    }
    if (std::abs(dist) <= TK::kSnapPointTolerance) {
        stage_plane_.distance += dist;
        UpdatePlaneWidgetPlane_();
        return true;
    }
    return false;
}

Plane ClipTool::GetStagePlaneFromModel_() const {
    // Convert to stage coordinates and then undo the centering translation.
    ASSERT(cm_);
    return TranslatePlane(
        TransformPlane(cm_->GetPlane(),
                       GetStageCoordConv().GetObjectToRootMatrix()),
        -cm_->GetLocalCenterOffset());
}

Plane ClipTool::GetStagePlaneFromWidget_() const {
    // Need to apply the current rotation and translation of the ClipTool to
    // the PlaneWidget's Plane. Since the ClipTool is never scaled, its model
    // matrix should do the trick.
    return TransformPlane(plane_widget_->GetPlane(), GetModelMatrix());
}

Plane ClipTool::StageToObjectPlane_(const Plane &stage_plane) const {
    // Use the distance from the PlaneWidget's plane, since that is based on
    // the actual plane translation.
    return Plane(plane_widget_->GetPlane().distance,
                 TransformNormal(stage_plane.normal,
                                 GetStageCoordConv().GetRootToObjectMatrix()));
}

void ClipTool::UpdatePlaneWidgetPlane_() {
    // This is the reverse of GetStagePlaneFromWidget_().
    plane_widget_->SetPlane(
        TransformPlane(stage_plane_, ion::math::Inverse(GetModelMatrix())));
}

void ClipTool::UpdateTranslationRange_() {
    // Compute the min/max signed distances of any vertex of the unclipped mesh
    // along the current clipping plane's normal vector. This assumes that the
    // PlaneWidget is centered on the unclipped mesh so that the mesh's center
    // point is at a distance of 0. Note that the mesh points need to be scaled
    // by the ClippedModel's scale to bring them into the object coordinates of
    // the ClipTool.
    ASSERT(cm_);
    const auto &mesh        = cm_->GetOperandModel()->GetMesh();
    const Vector3f &scale   = cm_->GetScale();
    float min_dist =  std::numeric_limits<float>::max();
    float max_dist = -std::numeric_limits<float>::max();
    const auto object_plane = StageToObjectPlane_(stage_plane_);
    for (const Point3f &p: mesh.points) {
        const float dist = SignedDistance(ScalePoint(p, scale),
                                          object_plane.normal);
        min_dist = std::min(min_dist, dist);
        max_dist = std::max(max_dist, dist);
    }

    // Set the range, making sure not to clip away all of the mesh by
    // restricting the minimum and maximum values.
    plane_widget_->SetTranslationRange(Range1f(min_dist + TK::kMinClippedSize,
                                               max_dist - TK::kMinClippedSize));
}

void ClipTool::UpdateTranslationFeedback_(const Color &color) {
    // Show the ClippedModel size in the direction of the plane normal. Find
    // the position of the minimum point (relative to the clipping plane) in
    // stage coordinates.
    ASSERT(cm_);
    const auto &mesh        = cm_->GetMesh();
    const Vector3f &scale   = cm_->GetScale();
    float min_dist = std::numeric_limits<float>::max();
    const auto object_plane = StageToObjectPlane_(stage_plane_);
    Point3f min_pt(0, 0, 0);
    for (const Point3f &p: mesh.points) {
        const Point3f scaled_pt = ScalePoint(p, scale);
        const float dist = SignedDistance(scaled_pt, object_plane.normal);
        if (dist < min_dist) {
            min_dist = dist;
            min_pt   = scaled_pt;
        }
    }
    const Point3f p0 = GetModelMatrix() * min_pt + cm_->GetLocalCenterOffset();
    const Point3f p1 = Point3f(stage_plane_.distance * stage_plane_.normal);
    feedback_->SetColor(color);
    feedback_->SpanLength(p0, stage_plane_.normal,
                          ion::math::Dot(p1 - p0, stage_plane_.normal));
}

void ClipTool::UpdateRealTimeClipPlane_(bool enable) {
    GetContext().root_model->EnableClipping(enable, stage_plane_);
}
