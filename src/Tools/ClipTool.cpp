#include "Tools/ClipTool.h"

#include "Managers/CommandManager.h"
#include "Models/ClippedModel.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Widgets/PushButtonWidget.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/SphereWidget.h"

// ----------------------------------------------------------------------------
// ClipTool::Parts_ struct.
// ----------------------------------------------------------------------------

/// This struct stores all of the parts the ClipTool needs to operate.
struct ClipTool::Parts_ {
    /// SphereWidget to rotate the clipping plane.
    SphereWidgetPtr     rotator;
    /// Arrow with a Slider1DWidget for translating the clipping plane.
    Slider1DWidgetPtr   arrow;
    /// PushButtonWidget plane for applying the clip plane.
    PushButtonWidgetPtr plane;

    /// Cylindrical shaft part of the arrow.
    SG::NodePtr         arrow_shaft;
    /// Conical end part of the arrow.
    SG::NodePtr         arrow_cone;

    // Scale factors for Widgets.
    static constexpr float kRotatorScale = 1.1f;
    static constexpr float kPlaneScale   = 1.5f;
    static constexpr float kArrowScale   = 1.6f;
};

// ----------------------------------------------------------------------------
// ClipTool functions.
// ----------------------------------------------------------------------------

ClipTool::ClipTool() {
}

void ClipTool::CreationDone() {
    Tool::CreationDone();

    if (! IsTemplate())
        FindParts_();
}

void ClipTool::UpdateGripInfo(GripInfo &info) {
    // XXXX
}

bool ClipTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<ClippedModel>(sel);
}

void ClipTool::Attach() {
    ASSERT(Util::IsA<ClippedModel>(GetModelAttachedTo()));
    ASSERT(parts_);
    UpdateGeometry_();
}

void ClipTool::Detach() {
    // Nothing to do here.
}

void ClipTool::FindParts_() {
    ASSERT(! parts_);
    parts_.reset(new Parts_);

    parts_->rotator =
        SG::FindTypedNodeUnderNode<SphereWidget>(*this, "Rotator");
    parts_->arrow =
        SG::FindTypedNodeUnderNode<Slider1DWidget>(*this, "Arrow");
    parts_->plane =
        SG::FindTypedNodeUnderNode<PushButtonWidget>(*this, "Plane");

    // Parts of the arrow
    parts_->arrow_shaft = SG::FindNodeUnderNode(*parts_->arrow, "Shaft");
    parts_->arrow_cone  = SG::FindNodeUnderNode(*parts_->arrow, "Cone");

    parts_->rotator->GetActivation().AddObserver(
        this, [&](Widget &, bool is_act){ RotatorActivated_(is_act); });
    parts_->rotator->GetRotationChanged().AddObserver(
        this, [&](Widget &, const Rotationf &){ Rotate_(); });

    // XXXX Set up interaction.
}

void ClipTool::UpdateGeometry_() {
    ASSERT(parts_);

    auto model = Util::CastToDerived<ClippedModel>(GetModelAttachedTo());
    ASSERT(model);

    // Note: no need to use isAxisAligned here, since that affects only
    // snapping.
    const Vector3f model_size = MatchModelAndGetSize(true);

    // Get the radius of the Model for scaling.
    const float radius = .5f * ion::math::Length(model_size);

    // Scale the plane and sphere relative to the model size.
    parts_->plane->SetUniformScale(Parts_::kPlaneScale * radius);
    parts_->rotator->SetUniformScale(Parts_::kRotatorScale * radius);

    // Scale the arrow shaft, move the arrow to the origin, and position the
    // cone at the end.
    const float arrow_scale = Parts_::kArrowScale * radius;
    parts_->arrow_shaft->SetScale(Vector3f(1, arrow_scale, 1));
    parts_->arrow_cone->SetTranslation(Vector3f(0, arrow_scale, 0));
    parts_->arrow->SetTranslation(Vector3f(0, .5f * arrow_scale, 0));

    // Match the last Plane in the ClippedModel if it has any. Otherwise, use
    // the XZ plane in object coordinates.
    if (! model->GetPlanes().empty())
        MatchPlane_(model->GetPlanes().back());
    else
        MatchPlane_(Plane(0, Vector3f::AxisY()));
}

void ClipTool::MatchPlane_(const Plane &plane) {
    // Use the plane from the Model's object coordinates, since the ClipTool is
    // set to match it.

    // Use the normal to set the rotation of the plane and arrow. The default
    // rotation for both is aligned with the +Y axis.
    const Rotationf rot = Rotationf::RotateInto(Vector3f::AxisY(),
                                                plane.normal);
    parts_->plane->SetRotation(rot);
    parts_->arrow->SetRotation(rot);

    // Use the distance of the plane from the center of the model to
    // translate the plane from the origin.
    parts_->plane->SetTranslation(-plane.distance * plane.normal);
    parts_->arrow->SetValue(-plane.distance);
}

void ClipTool::RotatorActivated_(bool is_activation) {
    UpdateRealTimeClipPlane_(is_activation);

    // XXXX Fix colors, anything else?
}

void ClipTool::Rotate_() {
    // Match the rotation of the SphereWidget.
    const Rotationf &rot = parts_->rotator->GetRotation();
    parts_->plane->SetRotation(rot);
    parts_->arrow->SetRotation(rot);

    UpdateRealTimeClipPlane_(true);
}

void ClipTool::UpdateRealTimeClipPlane_(bool enable) {
    // Convert the current clipping plane into stage coordinates.
    const auto &tool_plane = *parts_->plane;
    Plane plane(Point3f(tool_plane.GetTranslation()),
                tool_plane.GetRotation() * Vector3f::AxisY());
    plane = TransformPlane(plane, GetStageCoordConv().GetObjectToRootMatrix());
    GetContext().root_model->EnableClipping(enable, plane);
}
