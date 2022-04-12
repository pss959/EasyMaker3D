#include "Tools/ClipTool.h"

#include "Managers/CommandManager.h"
#include "Models/ClippedModel.h"
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

    // Scale factors for Widgets.
    static constexpr float kRotatorScale = 1.1f;
    static constexpr float kPlaneScale   = 1.5f;
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

    // Make the parts a little larger than the model size.
    parts_->plane->SetUniformScale(Parts_::kPlaneScale * radius);
    parts_->rotator->SetUniformScale(Parts_::kRotatorScale * radius);

    // Match the last Plane in the ClippedModel if it has any. Otherwise, use
    // the XZ plane in object coordinates.
    if (! model->GetPlanes().empty())
        MatchPlane_(model->GetPlanes().back());
    else
        MatchPlane_(Plane(0, Vector3f::AxisY()));
}

void ClipTool::MatchPlane_(const Plane &plane) {
    // Convert the plane from the Model's object coordinates to stage
    // coordinates.
    const Plane sp =
        TransformPlane(plane, GetStageCoordConv().GetObjectToRootMatrix());

    // Use the normal to set the rotation of the plane and arrow. The default
    // rotation for both is aligned with the +Y axis.
    const Rotationf rot = Rotationf::RotateInto(Vector3f::AxisY(), sp.normal);
    parts_->plane->SetRotation(rot);
    parts_->arrow->SetRotation(rot);

    // Use the distance of the plane from the center of the model to
    // translate the plane from the origin.
    parts_->plane->SetTranslation(-sp.distance * sp.normal);
    parts_->arrow->SetValue(-sp.distance);
}
