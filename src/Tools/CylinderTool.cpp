#include "Tools/CylinderTool.h"

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Math/Types.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Widgets/ScaleWidget.h"

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
    // XXXX
}

bool CylinderTool::CanAttachToModel(const Model &model) const {
    return dynamic_cast<const CylinderModel *>(&model);
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
    scaler->GetMinSlider().SetIsPrecisionBased(true);
    scaler->GetMaxSlider().SetIsPrecisionBased(true);

    // Scalar limits are for the full diameter.
    scaler->SetLimits(Vector2f(2 * kMinRadius_, 2 * kMaxRadius_));

    scaler->GetActivation().AddObserver(
        this, [&, scaler](Widget &, bool is_activation){
            ScalerActivated_(scaler, is_activation); });
    scaler->GetScaleChanged().AddObserver(
        this, [&, scaler](Widget &, bool is_max){
            ScalerChanged_(scaler, is_max); });
    scaler->GetScaleChanged().EnableObserver(this, false);

    return scaler;
}

/// Updates the scalers based on the attached CylinderModel.
void CylinderTool::UpdateScalers_() {
    ASSERT(cylinder_model_);
    const Vector3f size = MatchModelAndGetSize(false);

    // Update the radius scalers based on the current radii. Note that the
    // radii need to be converted from object coordinates to stage coordinates.
    const float scale = GetStageCoordConv().ObjectToRoot(Vector3f(1, 0, 0))[0];
    const float top_radius =
        scale * cylinder_model_->GetRadius(CylinderModel::Radius::kTop);
    const float bottom_radius =
        scale * cylinder_model_->GetRadius(CylinderModel::Radius::kBottom);
    top_scaler_->SetMinValue(-top_radius);
    top_scaler_->SetMaxValue( top_radius);
    bottom_scaler_->SetMinValue(-bottom_radius);
    bottom_scaler_->SetMaxValue( bottom_radius);

    // Position the scalers.
    Vector3f center_point(0, 0, 0);
    center_point[1] =  .5f * size[1];
    top_scaler_->SetTranslation(center_point);
    center_point[1] = -.5f * size[1];
    bottom_scaler_->SetTranslation(center_point);
}

void CylinderTool::ScalerActivated_(const ScaleWidgetPtr &scaler,
                                    bool is_activation) {
    ASSERT(cylinder_model_);

    if (is_activation) {
        // Starting a drag: activate the feedback.
        feedback_ = GetContext().feedback_manager->Activate<LinearFeedback>();
        feedback_->SetColor(Color::White());

        // Save the starting radius.
        const auto which = scaler == top_scaler_ ?
            CylinderModel::Radius::kTop : CylinderModel::Radius::kBottom;
        start_radius_ = cylinder_model_->GetRadius(which);

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
        // the transforms.
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
        const auto which = scaler == top_scaler_ ?
            CylinderModel::Radius::kTop : CylinderModel::Radius::kBottom;
        command_ = CreateCommand<ChangeCylinderCommand>("ChangeCylinderCommand");
        command_->SetFromSelection(GetSelection());
        command_->SetWhichRadius(which);
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
    } else if (target_manager.SnapToLength(2 * radius)) {  // Diameter snapped.
        radius = .5f * target_manager.GetEdgeTarget().GetLength();
        is_snapped = true;
    } else {
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
