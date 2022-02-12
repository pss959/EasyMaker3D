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

    scaler->SetLimits(Vector2f(kMinRadius_, kMaxRadius_));

    scaler->GetActivation().AddObserver(
        this, [&, scaler](Widget &, bool is_activation){
            ScalerActivated_(scaler, is_activation); });
    scaler->GetScaleChanged().AddObserver(
        this, [&, scaler](Widget &, bool is_max){
            ScalerChanged_(scaler, is_max); });

    return scaler;
}

/// Updates the scalers based on the attached CylinderModel.
void CylinderTool::UpdateScalers_() {
    ASSERT(cylinder_model_);
    const Vector3f size = MatchModelAndGetSize(false);

    // Update the radius scalers based on the current radii. Note that the
    // radii need to be converted from object coordinates.
    const float scale = cylinder_model_->GetScale()[0];
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
    center_point[1] = .5f * size[1];
    top_scaler_->SetTranslation(center_point);
    center_point[1] = -.5f * size[1];
    bottom_scaler_->SetTranslation(center_point);
}

void CylinderTool::ScalerActivated_(const ScaleWidgetPtr &scaler,
                                    bool is_activation) {
    ASSERT(cylinder_model_);

    const auto which = scaler == top_scaler_ ?
        CylinderModel::Radius::kTop : CylinderModel::Radius::kBottom;

    if (is_activation) {
        // Starting a drag: activate the feedback.
        feedback_ = GetContext().feedback_manager->Activate<LinearFeedback>();
        feedback_->SetColor(Color::White());

        // Save the starting radius.
        start_radius_ = cylinder_model_->GetRadius(which);
    }
    else {
        // This could be the end of a drag. If there was any change, execute
        // the command to change the transforms.
        if (command_) {
            if (command_->GetNewRadius() != start_radius_)
                GetContext().command_manager->AddAndDo(command_);
            command_.reset();
        }

        // Deactivate the feedback.
        GetContext().feedback_manager->Deactivate(feedback_);
        feedback_.reset();

        // Invoke the DragEnded callbacks.
        GetDragEnded().Notify(*this);
    }
}

void CylinderTool::ScalerChanged_(const ScaleWidgetPtr &scaler, bool is_max) {
    const auto which = scaler == top_scaler_ ?
        CylinderModel::Radius::kTop : CylinderModel::Radius::kBottom;

    // If this is the first change, create the ChangeCylinderCommand and start
    // the drag.
    if (! command_) {
        command_ = CreateCommand<ChangeCylinderCommand>("ChangeCylinderCommand");
        command_->SetFromSelection(GetSelection());
        command_->SetWhichRadius(which);
        GetDragStarted().Notify(*this);
    }

    // Try snapping both the stage-space radius and diameter to the current
    // target length.
    float radius = scaler->GetLength();

    auto &target_manager = *GetContext().target_manager;
    bool is_snapped;
    if (target_manager.SnapToLength(radius)) {
        radius = target_manager.GetEdgeTarget().GetLength();
        is_snapped = true;
    } else if (target_manager.SnapToLength(2 * radius)) {
        radius = .5f * target_manager.GetEdgeTarget().GetLength();
        is_snapped = true;
    } else {
        // Otherwise, apply precision to the new radius.
        radius = GetContext().precision_manager->Apply(radius) / radius;
        is_snapped = false;
    }
    // Convert the radius into the correct coordinates.
    radius = radius / cylinder_model_->GetScale()[0];

    // Update the command and simulate execution to update the Models.
    command_->SetNewRadius(radius);
    GetContext().command_manager->SimulateDo(command_);

    // Update the feedback using the motion vector.
    UpdateFeedback_(radius, is_snapped);
}

void CylinderTool::UpdateFeedback_(float radius, bool is_snapped) {
    const Matrix4f osm = GetObjectToStageMatrix();
    const Point3f  p0  = osm * Point3f(0, 1, 0);
    const Point3f  p1  = osm * Point3f(radius, 1, 0);
    const Vector3f dir = ion::math::Normalized(osm * Vector3f(1, 0, 0));

    // Use SpanLength() here instead of SpanPoints() because the length can
    // be zero.
    feedback_->SetColor(is_snapped ? GetSnappedFeedbackColor() : Color::White());
    feedback_->SpanLength(p0, dir, ion::math::Distance(p0, p1));
}
