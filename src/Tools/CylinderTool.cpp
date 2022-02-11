#include "Tools/CylinderTool.h"

#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Math/Types.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Widgets/RangeWidget.h"

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

RangeWidgetPtr CylinderTool::InitScaler_(const std::string &name) {
    auto scaler = SG::FindTypedNodeUnderNode<RangeWidget>(*this, name);

    scaler->SetMode(RangeWidget::Mode::kSymmetric);
    scaler->GetMinSlider().SetIsPrecisionBased(true);
    scaler->GetMaxSlider().SetIsPrecisionBased(true);
    scaler->GetActivation().AddObserver(
        this, [&, scaler](Widget &, bool is_act){ ScalerActivated_(scaler); });
    scaler->GetRangeChanged().AddObserver(
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
    float    top_radius = scale * cylinder_model_->GetTopRadius();
    float bottom_radius = scale * cylinder_model_->GetBottomRadius();
    top_scaler_->SetRange(Vector2f(-top_radius, top_radius));
    bottom_scaler_->SetRange(Vector2f(-bottom_radius, bottom_radius));

    // Position the scalers.
    Vector3f center_point(0, 0, 0);
    center_point[1] = .5f * size[1];
    top_scaler_->SetTranslation(center_point);
    center_point[1] = -.5f * size[1];
    bottom_scaler_->SetTranslation(center_point);
}

void CylinderTool::ScalerActivated_(const RangeWidgetPtr &scaler) {
    // XXXX
}

void CylinderTool::ScalerChanged_(const RangeWidgetPtr &scaler, bool is_max) {
    // XXXX
}

