#include "Tools/ClipTool.h"

#include <algorithm>
#include <limits>

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Feedback/LinearFeedback.h"
#include "Managers/ColorManager.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/PrecisionManager.h"
#include "Managers/TargetManager.h"
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
    /// Node containing both the arrow and the plane for rotating and
    /// positioning.
    SG::NodePtr         arrow_and_plane;

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

    /// Feedback showing translation distance.
    LinearFeedbackPtr   feedback;

    // Scale factors for Widgets.
    static constexpr float kRotatorScale = 1.1f;
    static constexpr float kPlaneScale   = 1.5f;
    static constexpr float kArrowScale   = 1.6f;


    /// Minimum amount of distance from the clipping plane to the min/max
    /// vertex in the plane normal direction so that the entire Model is not
    /// clipped away.
    static constexpr float kMinClipDistance = .01f;

    // Special colors.
    static const Color kDefaultArrowColor;
    static const Color kDefaultPlaneColor;

    /// Returns the current rotation in object coordinates.
    const Rotationf & GetRotation() const { return rotator->GetRotation(); }

    /// Returns the direction of the arrow in object coordinates.
    const Vector3f GetDirection() const {
        return GetRotation() * Vector3f::AxisY();
    }

    /// Returns the current Plane in object coordinates.
    const Plane GetPlane() const { return Plane(0, GetDirection()); }
};

const Color ClipTool::Parts_::kDefaultArrowColor{.9, .9, .8};
const Color ClipTool::Parts_::kDefaultPlaneColor{.9, .7, .8};

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

    parts_->arrow_and_plane = SG::FindNodeUnderNode(*this, "ArrowAndPlane");

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

    parts_->arrow->GetActivation().AddObserver(
        this, [&](Widget &, bool is_act){ TranslatorActivated_(is_act); });
    parts_->arrow->GetValueChanged().AddObserver(
        this, [&](Widget &, float){ Translate_(); });

    parts_->plane->GetClicked().AddObserver(
        this, [&](const ClickInfo &){ PlaneClicked_(); });

    UpdateColors_(Parts_::kDefaultArrowColor, Parts_::kDefaultPlaneColor);
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

    UpdateTranslationRange_();
}

void ClipTool::MatchPlane_(const Plane &plane) {
    // Use the plane from the Model's object coordinates, since the ClipTool is
    // set to match it.

    // Use the normal to set the rotation of the plane and arrow. The default
    // rotation for both is aligned with the +Y axis.
    parts_->arrow_and_plane->SetRotation(
        Rotationf::RotateInto(Vector3f::AxisY(), plane.normal));

    // Use the distance of the plane from the center of the model to
    // translate the plane from the origin.
    parts_->arrow_and_plane->SetTranslation(-plane.distance * plane.normal);
}

void ClipTool::RotatorActivated_(bool is_activation) {
    if (! is_activation) {
        UpdateColors_(Parts_::kDefaultArrowColor, Parts_::kDefaultPlaneColor);
        UpdateTranslationRange_();
    }
    UpdateRealTimeClipPlane_(is_activation);
}

void ClipTool::Rotate_() {
    parts_->arrow_and_plane->SetRotation(GetRotation_());
    UpdateRealTimeClipPlane_(true);
}

void ClipTool::TranslatorActivated_(bool is_activation) {
    const auto &context = GetContext();
    if (is_activation) {
        parts_->feedback = context.feedback_manager->Activate<LinearFeedback>();
        context.target_manager->StartSnapping();
    }
    else {
        context.target_manager->EndSnapping();
        context.feedback_manager->Deactivate(parts_->feedback);
        parts_->feedback.reset();
        UpdateColors_(Parts_::kDefaultArrowColor, Parts_::kDefaultPlaneColor);
        UpdateTranslationRange_();
    }

    // Hide the rotator sphere while translation is active.
    parts_->rotator->SetEnabled(! is_activation);
    UpdateRealTimeClipPlane_(is_activation);
}

void ClipTool::Translate_() {
#if XXXX
    using ion::math::Length;

    const auto &context = GetContext();
    const bool do_snapping = ! context.is_alternate_mode;

    // Get the motion direction vector and signed distance in object
    // coordinates.
    const Rotationf &rot = parts_->rotator->GetRotation();
    const Vector3f obj_dir = rot * Vector3f::AxisY();
    const float obj_distance = parts_->arrow->GetValue();
    const float sign = obj_distance < 0 ? -1 : 1;

    // Convert the motion vector to stage coordinates and get the signed
    // distance.
    const Matrix4f osm = GetStageCoordConv().GetObjectToRootMatrix();
    Vector3f stage_motion = osm * (obj_distance * obj_dir);
    float stage_distance = sign * Length(stage_motion);

    // Get the starting point in stage coordinates for snapping and feedback.
    const Point3f start_stage_pt = osm * Point3f::Zero();

    // Snap to important points if requested.
    bool is_snapped = false;
    if (do_snapping) {
        // Snap to the point target if it is active.
        if (context.target_manager->SnapToPoint(start_stage_pt, stage_motion)) {
            stage_distance = sign * Length(stage_motion);
            is_snapped = true;
        }
        // Otherwise, try snapping to the center of the Model.
        else if (std::abs(stage_distance) <= Defaults::kSnapPointTolerance) {
            stage_motion = Vector3f::Zero();
            stage_distance = 0;
            is_snapped = true;
        }
    }

    // Adjust by precision if requested and not snapped.
    if (do_snap && ! is_snapped)
        stage_distance = context.precision_manager->Apply(stage_distance);

    // Do not allow the plane to pass the min/max slider values.
    stage_distance = Clamp(stage_distance,
                           parts_->arrow->GetMinValue(),
                           parts_->arrow->GetMaxValue());
    stage_motion = stage_distance * obj_dir;

    // Convert back to local coordinates and translate in the default
    // direction (X). The rotation will be applied by Unity.
    Matrix4x4 slm = UT.GetStageToLocalMatrix(gameObject);
    float localDist = sign * slm.MultiplyVector(motion).magnitude;
    _planeGO.transform.localPosition = localDist * Vector3.right;

    Color color = isSnapped ? GetSnappedColor() : _StandardArrowColor;
    SetColors(color, color);

    if (_feedback != null) {
        _feedback.SetColor(color);
        _feedback.SpanLength(startPos, dir, distance);
    }
#endif
    const float distance = parts_->arrow->GetValue();
    const Vector3f motion = distance * parts_->GetDirection();

    // Match the translation of the Slider1DWidget.
    parts_->rotator->SetTranslation(motion);
    parts_->plane->SetTranslation(motion);

    UpdateRealTimeClipPlane_(true);
}

void ClipTool::PlaneClicked_() {
    // XXXX
}

Rotationf ClipTool::GetRotation_() {
    // Match the rotation of the SphereWidget, taking snapping into account.
    return parts_->rotator->GetRotation();

    // XXXX Deal with snapping and colors here...
}

void ClipTool::UpdateTranslationRange_() {
    auto model = GetModelAttachedTo();
    ASSERT(model);

    // Compute the min/max distances in object coordinates of any mesh vertex
    // from the plane parallel to the current clipping plane through the
    // Model's center. This assumes that the Model's mesh is centered on the
    // origin.
    const Plane plane = parts_->GetPlane();
    const auto &mesh = model->GetMesh();
    float min_dist =  std::numeric_limits<float>::max();
    float max_dist = -std::numeric_limits<float>::max();
    for (const Point3f &p: mesh.points) {
        const float dist = plane.GetDistanceToPoint(p);
        min_dist = std::min(min_dist, dist);
        max_dist = std::max(max_dist, dist);
    }

    // Make sure not to clip all of the Model.
    parts_->arrow->SetRange(min_dist + Parts_::kMinClipDistance,
                            max_dist - Parts_::kMinClipDistance);
}

void ClipTool::UpdateRealTimeClipPlane_(bool enable) {
    // Convert the current clipping plane into stage coordinates.
    const Plane plane = TransformPlane(
        parts_->GetPlane(), GetStageCoordConv().GetObjectToRootMatrix());
    GetContext().root_model->EnableClipping(enable, plane);
}

void ClipTool::UpdateColors_(const Color &plane_color,
                             const Color &arrow_color) {
    // XXXX
}
