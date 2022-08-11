#include "Tools/CylinderTool.h"

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Base/Tuning.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/PrecisionManager.h"
#include "Managers/TargetManager.h"
#include "Math/Types.h"
#include "SG/Search.h"
#include "Targets/EdgeTarget.h"
#include "Util/Assert.h"
#include "Widgets/ScaleWidget.h"
#include "Widgets/Slider1DWidget.h"

CylinderTool::CylinderTool() {
}

void CylinderTool::CreationDone() {
    Tool::CreationDone();

    if (! IsTemplate()) {
        top_scaler_    = InitScaler_("TopRadiusScaler");
        bottom_scaler_ = InitScaler_("BottomRadiusScaler");
        // The feedback is stored when activated.
    }
}

void CylinderTool::UpdateGripInfo(GripInfo &info) {
    // Convert the controller guide direction into coordinates of the Tool.
    const Vector3f guide_dir = -GetRotation() * info.guide_direction;

    // Choose the scaler to hover based on the controller's height relative to
    // the camera.
    const float cam_y = GetContext().camera_position[1];
    const auto &scaler = info.event.position3D[1] >= cam_y ?
        top_scaler_ : bottom_scaler_;

    // Use the scaler handle on the controller's side of the cylinder.
    info.widget = guide_dir[0] < 0 ?
        scaler->GetMaxSlider() : scaler->GetMinSlider();
    info.target_point = ToWorld(info.widget, Point3f::Zero());
}

bool CylinderTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<CylinderModel>(sel);
}

void CylinderTool::Attach() {
    ASSERT(Util::IsA<CylinderModel>(GetModelAttachedTo()));
    cylinder_model_ = Util::CastToDerived<CylinderModel>(GetModelAttachedTo());
    UpdateScalers_();
}

void CylinderTool::Detach() {
    cylinder_model_.reset();
}

ScaleWidgetPtr CylinderTool::InitScaler_(const std::string &name) {
    auto scaler = SG::FindTypedNodeUnderNode<ScaleWidget>(*this, name);

    scaler->SetMode(ScaleWidget::Mode::kSymmetric);
    scaler->GetMinSlider()->SetIsPrecisionBased(true);
    scaler->GetMaxSlider()->SetIsPrecisionBased(true);

    // Scaler limits are for the full diameter.
    scaler->SetLimits(Vector2f(2 * TK::kMinCylinderRadius,
                               2 * TK::kMaxCylinderRadius));

    scaler->GetActivation().AddObserver(
        this, [&, scaler](Widget &, bool is_activation){
            ScalerActivated_(scaler, is_activation); });
    scaler->GetScaleChanged().AddObserver(
        this, [&, scaler](Widget &, bool is_max){
            ScalerChanged_(scaler, is_max); });
    scaler->GetScaleChanged().EnableObserver(this, false);

    return scaler;
}

void CylinderTool::UpdateScalers_() {
    ASSERT(cylinder_model_);
    const Vector3f model_size = MatchModelAndGetSize(false);

    // Update the radius scalers based on the current radii. Note that the
    // radii need to be converted from object coordinates to stage coordinates.
    const float scale =
        ion::math::Length(GetStageCoordConv().ObjectToRoot(Vector3f::AxisX()));
    const float top_radius    = scale * cylinder_model_->GetTopRadius();
    const float bottom_radius = scale * cylinder_model_->GetBottomRadius();
    top_scaler_->SetMinValue(-top_radius);
    top_scaler_->SetMaxValue( top_radius);
    bottom_scaler_->SetMinValue(-bottom_radius);
    bottom_scaler_->SetMaxValue( bottom_radius);

    // Scale the handles and sticks based on the Model size.
    ScaleScaler_(*top_scaler_,    model_size);
    ScaleScaler_(*bottom_scaler_, model_size);

    // Position the scalers.
    Vector3f center_point(0, 0, 0);
    center_point[1] =  .5f * model_size[1];
    top_scaler_->SetTranslation(center_point);
    center_point[1] = -.5f * model_size[1];
    bottom_scaler_->SetTranslation(center_point);
}

void CylinderTool::ScaleScaler_(ScaleWidget &scaler,
                                const Vector3f &model_size) {
    const float kHandleSizeFraction = .25f;
    const float kMinHandleScale     = .1f;
    const float kMaxHandleScale     = .6f;
    const float handle_scale = Clamp(
        kHandleSizeFraction * model_size[GetMinElementIndex(model_size)],
        kMinHandleScale, kMaxHandleScale);

    // Scale the slider handles.
    SG::FindNodeUnderNode(scaler, "MinSlider")->SetUniformScale(handle_scale);
    SG::FindNodeUnderNode(scaler, "MaxSlider")->SetUniformScale(handle_scale);

    // Scale the stick (even though it is probably not visible).
    auto stick = SG::FindNodeUnderNode(scaler, "Stick");
    const float thickness_scale = .4f * handle_scale;
    stick->SetScale(Vector3f(stick->GetScale()[0],
                             thickness_scale, thickness_scale));
}

void CylinderTool::ScalerActivated_(const ScaleWidgetPtr &scaler,
                                    bool is_activation) {
    ASSERT(cylinder_model_);

    if (is_activation) {
        // Starting a drag: activate the feedback.
        feedback_ = GetContext().feedback_manager->Activate<LinearFeedback>();
        feedback_->SetColor(Color::White());

        // Save the starting radius.
        start_radius_ = scaler == top_scaler_ ? cylinder_model_->GetTopRadius() :
            cylinder_model_->GetBottomRadius();

        scaler->GetScaleChanged().EnableObserver(this, true);
    }
    else {
        scaler->GetScaleChanged().EnableObserver(this, false);

        // Invoke the DragEnded callbacks.
        GetDragEnded().Notify(*this);

        // Deactivate the feedback.
        GetContext().feedback_manager->Deactivate(feedback_);
        feedback_.reset();

        // If there was any change due to a drag, execute the command to change
        // the radius.
        if (command_) {
            if (command_->GetNewRadius() != start_radius_)
                GetContext().command_manager->AddAndDo(command_);
            command_.reset();
        }
    }
}

void CylinderTool::ScalerChanged_(const ScaleWidgetPtr &scaler, bool is_max) {
    // If this is the first change, create the ChangeCylinderCommand and start
    // the drag.
    if (! command_) {
        command_ = CreateCommand<ChangeCylinderCommand>();
        command_->SetFromSelection(GetSelection());
        command_->SetIsTopRadius(scaler == top_scaler_);
        GetDragStarted().Notify(*this);
    }

    // Try snapping both the stage-space radius and diameter to the current
    // target length. If that does not work, apply precision to the radius.
    float radius = .5f * scaler->GetLength();

    auto &target_manager = *GetContext().target_manager;
    bool is_snapped;
    if (target_manager.SnapToLength(radius)) {             // Radius snapped.
        radius = target_manager.GetEdgeTarget().GetLength();
        is_snapped = true;
    }
    else if (target_manager.SnapToLength(2 * radius)) {  // Diameter snapped.
        radius = .5f * target_manager.GetEdgeTarget().GetLength();
        is_snapped = true;
    }
    else {
        radius = GetContext().precision_manager->Apply(radius);
        is_snapped = false;
    }

    // Update the command and simulate execution to update the Models.
    command_->SetNewRadius(radius);
    GetContext().command_manager->SimulateDo(command_);

    // Update the feedback using the motion vector.
    UpdateFeedback_(radius, is_snapped);
}

void CylinderTool::UpdateFeedback_(float radius, bool is_snapped) {
    // Convert canonical points on the cylinder from object coordinates to
    // stage coordinates. Note that the radius is already in stage coordinates.
    const Matrix4f osm = GetStageCoordConv().GetObjectToRootMatrix();
    const Point3f  p0  = osm * Point3f(0, 1, 0);
    const Vector3f dir = ion::math::Normalized(osm * Vector3f(1, 0, 0));
    const Point3f  p1  = p0 + radius * dir;

    // Use SpanLength() here instead of SpanPoints() because the length can
    // be zero.
    feedback_->SetColor(is_snapped ? GetSnappedFeedbackColor() : Color::White());
    feedback_->SpanLength(p0, dir, ion::math::Distance(p0, p1));
}
