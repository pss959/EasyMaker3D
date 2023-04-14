#include "Tools/ClipTool.h"

// XXXX Check all of these
#include <algorithm>
#include <limits>

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
    const auto &cm = Util::CastToDerived<ClippedModel>(GetModelAttachedTo());
    ASSERT(cm);
    ASSERT(cm->GetOperandModel());

    // Rotate and translate to the ClippedModel.
    const auto model_size = MatchModelAndGetSize(true);

    // Update the widget size based on the model size.
    const float radius = .5f * ion::math::Length(model_size);
    plane_widget_->SetSize(radius);

    // Translate the PlaneWidget so that it is centered on the unclipped mesh.
    plane_widget_->SetTranslation(-cm->GetLocalCenterOffset());

    // Match the Plane in the ClippedModel. Note that this Plane does not
    // include the centering offset, so it is correct for the translated
    // PlaneWidget.
    plane_widget_->SetPlane(cm->GetPlane());

    // Update the range of the slider based on the size of the Model and the
    // normal direction.
    UpdateTranslationRange_();
}

void ClipTool::Detach() {
    // Nothing to do here.
}

void ClipTool::Activate_(bool is_activation) {
    const auto &context = GetContext();
    if (is_activation) {
        start_plane_ = plane_widget_->GetPlane();
        feedback_ = context.feedback_manager->Activate<LinearFeedback>();
        context.target_manager->StartSnapping();
        UpdateRealTimeClipPlane_(true, GetStagePlane_());
    }
    else {
        context.target_manager->EndSnapping();
        context.feedback_manager->Deactivate(feedback_);
        feedback_.reset();

        GetDragEnded().Notify(*this);
        GetContext().target_manager->EndSnapping();

        UpdateTranslationRange_();

        // If there was any change due to a drag, execute the command to change
        // the ClippedModel(s).
        if (command_) {
            if (! command_->GetPlane().IsDefault())
                GetContext().command_manager->AddAndDo(command_);
            command_.reset();
        }
        UpdateRealTimeClipPlane_(false, Plane());
    }
}

void ClipTool::PlaneChanged_(bool is_rotation) {
    // If this is the first change, create the ChangeClipCommand and start the
    // drag.
    if (! command_) {
        command_ = CreateCommand<ChangeClipCommand>();
        command_->SetFromSelection(GetSelection());
        GetDragStarted().Notify(*this);
    }

    const Plane stage_plane = GetStagePlane_();
    command_->SetPlane(stage_plane);
    GetContext().command_manager->SimulateDo(command_);

    UpdateRealTimeClipPlane_(true, stage_plane);

    // Update translation feedback.
    if (! is_rotation)
        UpdateTranslationFeedback_(Color(1, 0, 0, 1));  // XXXX Color
}

Plane ClipTool::GetStagePlane_() {
    // The PlaneWidget's plane is in the object coordinates of the ClipTool
    // except for the centering offset. Convert to stage coordinates and then
    // undo the centering translation.
    const auto &cm = Util::CastToDerived<ClippedModel>(GetModelAttachedTo());
    return TranslatePlane(
        TransformPlane(plane_widget_->GetPlane(),
                       GetStageCoordConv().GetObjectToRootMatrix()),
        -cm->GetLocalCenterOffset());
}

void ClipTool::UpdateTranslationRange_() {
    // Compute the min/max signed distances of any mesh vertex along the
    // current plane's normal vector. This assumes that the Model's mesh is
    // centered on the origin, so that the center point is at a distance of
    // 0. Convert the mesh points into stage coordinates so they can be
    // compared with the plane.
    // XXXX Fix this - need to use operand Model's mesh.
    const auto &model    = GetPrimary_();
    const auto &mesh     = model.GetMesh();
    const auto &normal   = plane_widget_->GetPlane().normal;
    float       min_dist =  std::numeric_limits<float>::max();
    float       max_dist = -std::numeric_limits<float>::max();
    const auto  stage_cc = GetStageCoordConv();
    for (const Point3f &p: mesh.points) {
        const float dist = SignedDistance(stage_cc.ObjectToRoot(p), normal);
        min_dist = std::min(min_dist, dist);
        max_dist = std::max(max_dist, dist);
    }

    // Set the translation range, making sure not to clip away all of the mesh
    // by restricting the minimum and maximum values.
    plane_widget_->SetTranslationRange(Range1f(min_dist + TK::kMinClippedSize,
                                               max_dist - TK::kMinClippedSize));

    // XXXX TEMPORARY!
    plane_widget_->SetTranslationRange(Range1f(-20, 20));
}

void ClipTool::UpdateTranslationFeedback_(const Color &color) {
    // XXXX Consider showing something else? Model size?

    // Need signed distance in stage coordinates.
    const auto &current_plane = plane_widget_->GetPlane();
    const float distance = current_plane.distance - start_plane_.distance;

    feedback_->SetColor(color);
    feedback_->SpanLength(Point3f::Zero(), current_plane.normal, distance); // XXXX pos
}

void ClipTool::UpdateRealTimeClipPlane_(bool enable, const Plane &stage_plane) {
    GetContext().root_model->EnableClipping(enable, stage_plane);
}

ClippedModel & ClipTool::GetPrimary_() const {
    const auto primary = GetSelection().GetPrimary().GetModel();
    const auto &cm = Util::CastToDerived<ClippedModel>(primary);
    ASSERT(cm);
    return *cm;
}
