#include "Tools/TorusTool.h"

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Base/Tuning.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/PrecisionManager.h"
#include "Managers/TargetManager.h"
#include "Math/Linear.h"
#include "Math/Types.h"
#include "SG/Search.h"
#include "Targets/EdgeTarget.h"
#include "Util/Assert.h"
#include "Widgets/ScaleWidget.h"
#include "Widgets/Slider1DWidget.h"

TorusTool::TorusTool() {
}

void TorusTool::CreationDone() {
    Tool::CreationDone();

    if (! IsTemplate()) {
        inner_scaler_ = InitScaler_("InnerRadiusScaler", true);
        outer_scaler_ = InitScaler_("OuterRadiusScaler", false);
        // The feedback is stored when activated.
    }
}

void TorusTool::UpdateGripInfo(GripInfo &info) {
    // Convert the controller guide direction into coordinates of the Tool.
    const Vector3f guide_dir = -GetRotation() * info.guide_direction;

    // Hover either scaler if the direction is close to its direction. The
    // outer radius scaler is along the X axis and the inner radius scaler is
    // along the Y axis.
    if (AreDirectionsClose(guide_dir, Vector3f::AxisX(),
                           TK::kMaxGripHoverDirAngle))
        info.widget = outer_scaler_->GetMinSlider();
    else if (AreDirectionsClose(guide_dir, -Vector3f::AxisX(),
                                TK::kMaxGripHoverDirAngle))
        info.widget = outer_scaler_->GetMaxSlider();
    else if (AreDirectionsClose(guide_dir, Vector3f::AxisY(),
                                TK::kMaxGripHoverDirAngle))
        info.widget = inner_scaler_->GetMinSlider();
    else if (AreDirectionsClose(guide_dir, -Vector3f::AxisY(),
                                TK::kMaxGripHoverDirAngle))
        info.widget = inner_scaler_->GetMaxSlider();
    if (info.widget)
        info.target_point = ToWorld(info.widget, Point3f::Zero());
}

bool TorusTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<TorusModel>(sel);
}

void TorusTool::Attach() {
    ASSERT(Util::IsA<TorusModel>(GetModelAttachedTo()));
    torus_model_ = Util::CastToDerived<TorusModel>(GetModelAttachedTo());
    UpdateScalers_();
}

void TorusTool::Detach() {
    torus_model_.reset();
}

ScaleWidgetPtr TorusTool::InitScaler_(const std::string &name, bool is_inner) {
    auto scaler = SG::FindTypedNodeUnderNode<ScaleWidget>(*this, name);

    scaler->SetMode(ScaleWidget::Mode::kSymmetric);
    scaler->GetMinSlider()->SetIsPrecisionBased(true);
    scaler->GetMaxSlider()->SetIsPrecisionBased(true);

    const float min = is_inner ? TK::kMinTorusInnerRadius :
        TorusModel::GetMinOuterRadiusForInnerRadius(TK::kMinTorusInnerRadius);
    const float max =
        is_inner ? TK::kMaxTorusInnerRadius : TK::kMaxTorusOuterRadius;
    scaler->SetLimits(Vector2f(2 * min, 2 * max));

    scaler->GetActivation().AddObserver(
        this, [&, scaler](Widget &, bool is_activation){
            ScalerActivated_(scaler, is_activation); });
    scaler->GetScaleChanged().AddObserver(
        this, [&, scaler](Widget &, bool is_max){
            ScalerChanged_(scaler, is_max); });
    scaler->GetScaleChanged().EnableObserver(this, false);

    return scaler;
}

void TorusTool::UpdateScalers_() {
    ASSERT(torus_model_);
    const Vector3f size = MatchModelAndGetSize(false);

    // Update the radius scalers based on the current radii. Note that the
    // radii need to be converted from object coordinates to stage coordinates.
    const float scale =
        ion::math::Length(GetStageCoordConv().ObjectToRoot(Vector3f::AxisX()));
    const float inner_radius = scale * torus_model_->GetInnerRadius();
    const float outer_radius = scale * torus_model_->GetOuterRadius();
    inner_scaler_->SetMinValue(-inner_radius);
    inner_scaler_->SetMaxValue( inner_radius);
    outer_scaler_->SetMinValue(-outer_radius);
    outer_scaler_->SetMaxValue( outer_radius);

    // Position the inner radius scaler. The outer radius scaler remains
    // centered on the center of the TorusModel. The inner radius scaler is
    // centered on the circular cross-section at the +X end.
    inner_scaler_->SetTranslation(Vector3f(.5f * size[0] - inner_radius, 0, 0));
}

void TorusTool::ScalerActivated_(const ScaleWidgetPtr &scaler,
                                 bool is_activation) {
    ASSERT(torus_model_);

    const bool is_inner = scaler == inner_scaler_;
    const ScaleWidgetPtr other_scaler =
        is_inner ? outer_scaler_ : inner_scaler_;

    if (is_activation) {
        // Starting a drag: activate the feedback.
        feedback_ = GetContext().feedback_manager->Activate<LinearFeedback>();
        feedback_->SetColor(Color::White());

        // Save the starting radii.
        start_radius_ = is_inner ? torus_model_->GetInnerRadius() :
            torus_model_->GetOuterRadius();
        start_outer_radius_ = torus_model_->GetOuterRadius();

        scaler->GetScaleChanged().EnableObserver(this, true);

        // Hide the other scaler.
        other_scaler->SetEnabled(false);
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

        // Show the other scaler.
        other_scaler->SetEnabled(true);
    }
}

void TorusTool::ScalerChanged_(const ScaleWidgetPtr &scaler, bool is_max) {
    const bool is_inner = scaler == inner_scaler_;

    // If this is the first change, create the ChangeTorusCommand and start
    // the drag.
    if (! command_) {
        command_ = CreateCommand<ChangeTorusCommand>();
        command_->SetFromSelection(GetSelection());
        command_->SetIsInnerRadius(is_inner);
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

    // If the outer radius grew because of a change to the inner radius,
    // restore the inner radius as much as possible when it gets smaller.
    if (is_inner) {
        const float r = std::max(torus_model_->GetMinOuterRadius(),
                                 start_outer_radius_);
        if (torus_model_->GetOuterRadius() > r)
            torus_model_->SetOuterRadius(r);
    }

    // Update the feedback using the motion vector.
    UpdateFeedback_(radius, is_snapped);
}

void TorusTool::UpdateFeedback_(float radius, bool is_snapped) {
    // Convert canonical points on the torus from object coordinates to
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
