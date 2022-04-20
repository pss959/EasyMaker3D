#include "Tools/ClipTool.h"

#include <algorithm>
#include <functional>
#include <limits>

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Commands/ChangeClipCommand.h"
#include "Feedback/LinearFeedback.h"
#include "Managers/ColorManager.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/PrecisionManager.h"
#include "Managers/TargetManager.h"
#include "Models/ClippedModel.h"
#include "Models/RootModel.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Widgets/PushButtonWidget.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/SphereWidget.h"

// ----------------------------------------------------------------------------
// ClipTool::Impl_ class.
// ----------------------------------------------------------------------------

class ClipTool::Impl_ {
  public:
    typedef std::function<void(const Plane &obj_plane)> ClipFunc;
    typedef std::function<void(bool enable, const Plane &obj_plane)> RTClipFunc;

    Impl_(const Tool::Context &context, const SG::Node &root_node);
    void AttachToClippedModel(const ClippedModelPtr &model,
                              const Vector3f &model_size,
                              const SG::NodePath &stage_path);

    /// Sets a function to invoke when the plane button is clicked to add a
    /// clipping plane. It is passed the clipping plane in object coordinates.
    void SetClipFunc(const ClipFunc &func) { clip_func_ = func; }

    /// Sets a function to invoke to enable or disable real-time clipping to
    /// the given plane in object coordinates.
    void SetRealTimeClipFunc(const RTClipFunc &func) { rt_clip_func_ = func; }

  private:
    const Tool::Context &context_;
    ClippedModelPtr      model_;
    SG::NodePath         stage_path_;
    ClipFunc             clip_func_;
    RTClipFunc           rt_clip_func_;

    /// Node containing both the arrow and the plane. This is rotated to match
    /// the current plane, including during SphereWidget interaction.
    SG::NodePtr         arrow_and_plane_;

    /// Interactive SphereWidget to rotate the clipping plane. The rotation
    /// in this defines the clipping plane normal.
    SphereWidgetPtr     rotator_;

    /// Arrow with a Slider1DWidget for translating the clipping plane. The min
    /// and max values are computed to stay within the ClippedModel's mesh and
    /// the current value defines the signed distance of the clipping plane
    /// from the center of the ClippedModel.
    Slider1DWidgetPtr   arrow_;

    /// PushButtonWidget plane for applying the clip plane. This is translated
    /// during Slider1DWidget interaction to show the current plane position.
    PushButtonWidgetPtr plane_;

    /// Cylindrical shaft part of the arrow. This is scaled in Y based on the
    /// size of the ClippedModel.
    SG::NodePtr         arrow_shaft_;

    /// Conical end part of the arrow. This is translated in Y to stay at the
    /// end of the arrow_shaft.
    SG::NodePtr         arrow_cone_;

    /// Feedback showing translation distance.
    LinearFeedbackPtr   feedback_;

    // Scale factors for Widgets.
    static constexpr float kRotatorScale_ = 1.1f;
    static constexpr float kPlaneScale_   = 1.5f;
    static constexpr float kArrowScale_   = 1.6f;  // Larger than sqrt(2).

    /// Minimum amount of distance from the clipping plane to the min/max
    /// vertex in the plane normal direction so that the entire ClippedModel is
    /// not clipped away.
    static constexpr float kMinClipDistance_ = .01f;

    /// Default color to use when the arrow is not snapped.
    static const Color kDefaultArrowColor_;

    /// Sets up to match the given Plane.
    void MatchPlane_(const Plane &plane);

    /// Sets the min/max range for the translation slider based on the Model's
    /// mesh extents along the given direction vector.
    void UpdateTranslationRange_(const Vector3f &dir);

    // Widget callbacks.
    void RotatorActivated_(bool is_activation);
    void Rotate_();
    void TranslatorActivated_(bool is_activation);
    void Translate_();
    void PlaneClicked_();

    /// Checks for snapping of the given rotation to the target and principal
    /// axes. If snapping occurs, this modifies rot and sets snapped_to_target
    /// or snapped_dim appropriately.
    void SnapRotation_(Rotationf &rot, bool &snapped_to_target,
                       int &snapped_dim);

    /// Checks for snapping of the given plane distance to an important point
    /// (point target or Model center). If snapping occurs, this modifies
    /// distance and sets is_snapped to true.
    void SnapTranslation_(float &distance, bool &is_snapped);

    /// Updates the state of the real-time clipping plane implemented in the
    /// Faceted shader.
    void UpdateRealTimeClipPlane_(bool enable);

    /// Returns the current clipping plane in object coordinates.
    Plane GetObjPlane_() const;
};

const Color ClipTool::Impl_::kDefaultArrowColor_{.9, .9, .8};

ClipTool::Impl_::Impl_(const Tool::Context &context,
                       const SG::Node &root_node) :
    context_(context) {

    // Find all parts
    arrow_and_plane_ = SG::FindNodeUnderNode(root_node, "ArrowAndPlane");
    rotator_ = SG::FindTypedNodeUnderNode<SphereWidget>(root_node, "Rotator");
    arrow_   = SG::FindTypedNodeUnderNode<Slider1DWidget>(root_node, "Arrow");
    plane_   = SG::FindTypedNodeUnderNode<PushButtonWidget>(root_node, "Plane");

    // Parts of the arrow
    arrow_shaft_ = SG::FindNodeUnderNode(*arrow_, "Shaft");
    arrow_cone_  = SG::FindNodeUnderNode(*arrow_, "Cone");

    rotator_->GetActivation().AddObserver(
        this, [&](Widget &, bool is_act){ RotatorActivated_(is_act); });
    rotator_->GetRotationChanged().AddObserver(
        this, [&](Widget &, const Rotationf &){ Rotate_(); });

    arrow_->GetActivation().AddObserver(
        this, [&](Widget &, bool is_act){ TranslatorActivated_(is_act); });
    arrow_->GetValueChanged().AddObserver(
        this, [&](Widget &, float){ Translate_(); });

    plane_->GetClicked().AddObserver(
        this, [&](const ClickInfo &){ PlaneClicked_(); });

    arrow_->SetInactiveColor(kDefaultArrowColor_);
}

void ClipTool::Impl_::AttachToClippedModel(const ClippedModelPtr &model,
                                           const Vector3f &model_size,
                                           const SG::NodePath &stage_path) {
    ASSERT(model);
    model_ = model;
    stage_path_ = stage_path;

    // Update sizes based on the model size.
    const float radius = .5f * ion::math::Length(model_size);
    plane_->SetUniformScale(kPlaneScale_ * radius);
    rotator_->SetUniformScale(kRotatorScale_ * radius);

    // Scale the arrow shaft and position the cone at the end.
    const float arrow_scale = kArrowScale_ * radius;
    arrow_shaft_->SetScale(Vector3f(1, arrow_scale, 1));
    arrow_cone_->SetTranslation(Vector3f(0, arrow_scale, 0));

    // Match the last Plane in the ClippedModel (converted to local
    // coordinates) if it has any. Otherwise, use the XZ plane in object
    // coordinates.
    if (! model_->GetPlanes().empty())
        MatchPlane_(TransformPlane(model_->GetPlanes().back(),
                                   model_->GetModelMatrix()));
    else
        MatchPlane_(Plane(0, Vector3f::AxisY()));
}

void ClipTool::Impl_::MatchPlane_(const Plane &plane) {
    // This uses the plane in the ClippedModel's object coordinates, since the
    // ClipTool's transformation is set to match it.

    // Use the plane normal to compute the rotation. The default object arrow
    // direction is the +Y axis.
    const Rotationf rot =
        Rotationf::RotateInto(Vector3f::AxisY(), plane.normal);
    rotator_->SetRotation(rot);
    arrow_and_plane_->SetRotation(rot);

    // Update the range of the slider based on the size of the Model and the
    // rotated normal direction.
    UpdateTranslationRange_(plane.normal);

    // Use the distance of the plane as the Slider1DWidget value without
    // notifying.
    arrow_->GetValueChanged().EnableObserver(this, false);
    arrow_->SetValue(plane.distance);
    arrow_->GetValueChanged().EnableObserver(this, true);

    // Position the plane at the correct distance.
    plane_->SetTranslation(arrow_->GetValue() * Vector3f::AxisY());
}

void ClipTool::Impl_::UpdateTranslationRange_(const Vector3f &dir) {
    ASSERT(model_);

    // Compute the min/max signed distances in object coordinates of any mesh
    // vertex along the dir vector. This assumes that the Model's mesh is
    // centered on the origin, so that the center point is at a distance of 0.
    const auto &mesh = model_->GetMesh();
    float min_dist =  std::numeric_limits<float>::max();
    float max_dist = -std::numeric_limits<float>::max();
    for (const Point3f &p: mesh.points) {
        const float dist = SignedDistance(p, dir);
        min_dist = std::min(min_dist, dist);
        max_dist = std::max(max_dist, dist);
    }

    // Set the range, making sure not to clip away all of the mesh. Restrict
    // the maximum only if this is the first clipping plane in the Model.
    const bool limit_max = model_->GetPlanes().empty();
    arrow_->SetRange(min_dist + kMinClipDistance_,
                     max_dist - (limit_max ? kMinClipDistance_ : 0));
}

void ClipTool::Impl_::RotatorActivated_(bool is_activation) {
    if (! is_activation) {
        arrow_->SetInactiveColor(kDefaultArrowColor_);

        // The rotation of the SphereWidget and the arrow+plane may differ. Use
        // the arrow+plane rotation, which includes snapping.
        const Rotationf &rot = arrow_and_plane_->GetRotation();
        rotator_->SetRotation(rot);

        // The normal may have changed during rotation, so update the
        // translation range now that it is done.
        const Vector3f dir = rot * Vector3f::AxisY();
        UpdateTranslationRange_(dir);
    }
    UpdateRealTimeClipPlane_(is_activation);
}

void ClipTool::Impl_::Rotate_() {
    // Get the current rotation.
    Rotationf rot = rotator_->GetRotation();

    // Snap if requested.
    bool snapped_to_target = false;
    int  snapped_dim = -1;
    if (! context_.is_alternate_mode)
        SnapRotation_(rot, snapped_to_target, snapped_dim);

    arrow_->SetInactiveColor(
        snapped_to_target ? GetSnappedFeedbackColor() :
        snapped_dim >= 0  ? ColorManager::GetColorForDimension(snapped_dim) :
        kDefaultArrowColor_);

    // Rotate the arrow and plane geometry to match.
    arrow_and_plane_->SetRotation(rot);

    UpdateRealTimeClipPlane_(true);
}

void ClipTool::Impl_::TranslatorActivated_(bool is_activation) {
    if (is_activation) {
        feedback_ = context_.feedback_manager->Activate<LinearFeedback>();
        context_.target_manager->StartSnapping();
    }
    else {
        context_.target_manager->EndSnapping();
        context_.feedback_manager->Deactivate(feedback_);
        feedback_.reset();
        arrow_->SetInactiveColor(kDefaultArrowColor_);
    }

    // Hide the rotator sphere while translation is active.
    rotator_->SetEnabled(! is_activation);

    UpdateRealTimeClipPlane_(is_activation);
}

void ClipTool::Impl_::Translate_() {
    // Get the current signed plane distance.
    float distance = arrow_->GetValue();

    // Snap if requested.
    bool is_snapped = false;
    if (! context_.is_alternate_mode)
        SnapTranslation_(distance, is_snapped);

    arrow_->SetActiveColor(is_snapped ? GetSnappedFeedbackColor() :
                           ColorManager::GetSpecialColor("WidgetActiveColor"));

    // Move the plane to the correct distance.
    plane_->SetTranslation(distance * Vector3f::AxisY());

    UpdateRealTimeClipPlane_(true);

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
}

void ClipTool::Impl_::PlaneClicked_() {
    if (clip_func_)
        clip_func_(GetObjPlane_());
}

void ClipTool::Impl_::SnapRotation_(Rotationf &rot, bool &snapped_to_target,
                                    int &snapped_dim) {
    auto &tm = *context_.target_manager;
    CoordConv cc(stage_path_);

    // Get the rotated plane normal in local and stage coordinates.
    Vector3f local_normal = rot * Vector3f::AxisY();
    Vector3f stage_normal =
        ion::math::Normalized(cc.ObjectToRoot(local_normal));

    auto get_local_snap_rot = [&](const Rotationf &rot){
        return Rotationf::RotateInto(Vector3f::AxisY(), rot * local_normal);
    };
    auto get_stage_snap_rot = [&](const Rotationf &rot){
        return Rotationf::RotateInto(Vector3f::AxisY(),
                                     cc.RootToObject(rot * stage_normal));
    };

    // Try to snap to the point target (in stage coordinates) if it is active.
    Rotationf snap_rot;
    if (tm.SnapToDirection(stage_normal, snap_rot)) {
        snapped_to_target = true;
        rot = get_stage_snap_rot(snap_rot);
    }

    // Otherwise, try to snap to any of the principal axes. Use stage axes if
    // is_axis_aligned is true; otherwise, use local axes.
    else {
        const bool use_stage = context_.is_axis_aligned;
        const Vector3f dir = use_stage ? stage_normal : local_normal;
        for (int dim = 0; dim < 3; ++dim) {
            Vector3f axis = GetAxis(dim);
            if (tm.ShouldSnapDirections(dir,  axis, snap_rot) ||
                tm.ShouldSnapDirections(dir, -axis, snap_rot)) {
                snapped_dim = dim;
                rot = use_stage ? get_stage_snap_rot(snap_rot) :
                    get_local_snap_rot(snap_rot);
                break;
            }
        }
    }
}

void ClipTool::Impl_::SnapTranslation_(float &distance, bool &is_snapped) {
    // XXXX Do something.
    if (std::abs(distance) < 1) { // XXXX TEMPORARY
        distance = 0;
        is_snapped = true;
    }
}

void ClipTool::Impl_::UpdateRealTimeClipPlane_(bool enable) {
    if (rt_clip_func_)
        rt_clip_func_(enable, GetObjPlane_());
}

Plane ClipTool::Impl_::GetObjPlane_() const {
    // Use the arrow+plane rotation and the plane translation, since snapping
    // may be in progress.
    const Vector3f normal = arrow_and_plane_->GetRotation() * Vector3f::AxisY();
    const float    distance = plane_->GetTranslation()[1];
    return Plane(distance, normal);
}

// ----------------------------------------------------------------------------
// ClipTool functions.
// ----------------------------------------------------------------------------

ClipTool::ClipTool() {
}

void ClipTool::CreationDone() {
    Tool::CreationDone();
}

void ClipTool::UpdateGripInfo(GripInfo &info) {
    // XXXX
}

bool ClipTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<ClippedModel>(sel);
}

void ClipTool::Attach() {
    ASSERT(Util::IsA<ClippedModel>(GetModelAttachedTo()));

    if (! impl_) {
        impl_.reset(new Impl_(GetContext(), *this));
        impl_->SetClipFunc(
            [&](const Plane &obj_plane){ AddPlane_(obj_plane); });
        impl_->SetRealTimeClipFunc(
            [&](bool enable, const Plane &obj_plane){
            GetContext().root_model->EnableClipping(
                enable, GetStagePlane_(obj_plane)); });
    }

    // Match the ClippedModel's transform and pass the size of the Model for
    // scaling. Note: no need to use isAxisAligned here, since that affects
    // only snapping.
    const Vector3f model_size = MatchModelAndGetSize(true);
    impl_->AttachToClippedModel(
        Util::CastToDerived<ClippedModel>(GetModelAttachedTo()), model_size,
        GetSelection().GetPrimary());
}

void ClipTool::Detach() {
    // Nothing to do here.
}

void ClipTool::AddPlane_(const Plane &obj_plane) {
    auto command = CreateCommand<ChangeClipCommand>();
    command->SetFromSelection(GetSelection());
    command->SetPlane(GetStagePlane_(obj_plane));
    GetContext().command_manager->AddAndDo(command);
}

Plane ClipTool::GetStagePlane_(const Plane &obj_plane) {
    return TransformPlane(obj_plane,
                          GetStageCoordConv().GetObjectToRootMatrix());
}
