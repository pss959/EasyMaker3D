#include "Tools/ClipTool.h"

#include <algorithm>
#include <functional>
#include <limits>

#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Commands/ChangeClipCommand.h"
#include "Feedback/LinearFeedback.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/TargetManager.h"
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
#include "Widgets/PushButtonWidget.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/SphereWidget.h"

// ----------------------------------------------------------------------------
// ClipTool::Impl_ class.
// ----------------------------------------------------------------------------

/// Implements the ClipTool. The tool consists of a SphereWidget for plane
/// rotation, a Slider1DWidget (arrow) for plane translation, and a
/// PushButtonWidget (plane) to apply the clipping plane.  The tool is rotated
/// and translated to align with the primary selection (ClippedModel) and the
/// widgets are scaled to surround it.
///
/// The current clipping plane is stored as current_plane_ in the object
/// coordinates of the ClipTool. This plane is converted to stage coordinates
/// for the ChangeClipCommand and for real-time clipping.
class ClipTool::Impl_ {
  public:
    Impl_(const Tool::Context &context, const SG::Node &root_node);

    void Attach(const Selection &sel, const Vector3f &model_size);
    void Detach();

    /// Implements UpdateGripInfo() for the ClipTool. Returns a Node that
    /// should be used to compute the target point.
    SG::NodePtr UpdateGripInfo(GripInfo &info, const Vector3f &guide_dir);

  private:
    const Tool::Context &context_;

    /// The ClipTool as a Node, which is used for coordinate conversions.
    const SG::Node      &root_node_;

    /// This stores the current selection (should be all ClippedModels).
    Selection           selection_;

    /// Current clipping plane in the object coordinates of the ClipTool.
    Plane               object_plane_{0, Vector3f::AxisY()};

    /// Current clipping plane in stage coordinates.
    Plane               stage_plane_{0, Vector3f::AxisY()};

    /// Node containing both the arrow and the plane. This is rotated to match
    /// the current plane, including during SphereWidget interaction.
    SG::NodePtr         arrow_and_plane_;

    /// Interactive SphereWidget to rotate the clipping plane. Its rotation
    /// defines the clipping plane normal.
    SphereWidgetPtr     rotator_;

    /// Arrow with a Slider1DWidget for translating the clipping plane. The min
    /// and max values are computed to stay within the ClippedModel's mesh and
    /// the current value defines the signed distance of the clipping plane
    /// from the center of the mesh in object coordinates of the tool.
    Slider1DWidgetPtr   arrow_;

    /// PushButtonWidget plane for applying the clip plane. This is translated
    /// along the Y axis to match the current plane position.
    PushButtonWidgetPtr plane_;

    /// Cylindrical shaft part of the arrow. This is scaled in Y based on the
    /// size of the ClippedModel.
    SG::NodePtr         arrow_shaft_;

    /// Conical end part of the arrow. This is translated in Y to stay at the
    /// end of the arrow_shaft.
    SG::NodePtr         arrow_cone_;

    /// Distance of the plane at the start of translation in local coordinates.
    float               start_distance_ = 0;

    /// Feedback showing translation distance in local coordinates.
    LinearFeedbackPtr   feedback_;

    /// Saves the color used for the Arrow when inactive so it can be changed.
    Color               arrow_inactive_color_;

    /// Updates all geometry to match the object_plane_.
    void MatchPlane_();

    /// Sets the min/max range for the translation slider based on the Model's
    /// mesh extents along the current plane's normal.
    void UpdateTranslationRange_();

    // Widget callbacks.
    void RotatorActivated_(bool is_activation);
    void Rotate_();
    void TranslatorActivated_(bool is_activation);
    void Translate_();
    void PlaneClicked_();

    /// Checks for snapping of the current plane's normal to the target and
    /// principal axes. If snapping occurs, this modifies the object and stage
    /// planes and sets snapped_to_target or snapped_dim appropriately.
    void SnapRotation_(bool &snapped_to_target, int &snapped_dim);

    /// Checks for snapping of the current plane's distance to an important
    /// point (point target or Model center). If snapping occurs, this modifies
    /// the object and stage planes and sets is_snapped to true.
    void SnapTranslation_(bool &is_snapped);

    /// Returns the rotation of the current plane in object coordinates.
    Rotationf GetPlaneRotation_() const {
        return Rotationf::RotateInto(Vector3f::AxisY(), object_plane_.normal);
    }

    /// Updates stage_plane_ from object_plane_.
    void UpdateStagePlane_();

    /// Updates object_plane_ from stage_plane_.
    void UpdateObjectPlane_();

    /// Updates feedback for plane translation.
    void UpdateTranslationFeedback_(const Color &color);

    /// Updates the state of the real-time clipping plane implemented in the
    /// Faceted shader.
    void UpdateRealTimeClipPlane_(bool enable);

    /// Returns the indexed selected ClippedModel. Asserts if the index is bad
    /// or the Model is the wrong type.
    ClippedModel & GetModel_(size_t index) const {
        ASSERT(index < selection_.GetCount());
        const auto &model = selection_.GetPaths()[index].GetModel();
        const auto &cm = Util::CastToDerived<ClippedModel>(model);
        ASSERT(cm);
        return *cm;
    }
};

ClipTool::Impl_::Impl_(const Tool::Context &context,
                       const SG::Node &root_node) :
    context_(context),
    root_node_(root_node) {

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

    arrow_inactive_color_ = arrow_->GetInactiveColor();
}

void ClipTool::Impl_::Attach(const Selection &sel, const Vector3f &model_size) {
    using ion::math::Dot;
    using ion::math::Inverse;
    using ion::math::ScaleMatrixH;

    selection_ = sel;

    // Update sizes based on the model size.
    const float kArrowScale   = 1.6f;  // Must be > sqrt(2).
    const float kPlaneScale   = 1.5f;
    const float kRotatorScale = 1.1f;

    const float radius = .5f * ion::math::Length(model_size);
    plane_->SetUniformScale(kPlaneScale * radius);
    rotator_->SetUniformScale(kRotatorScale * radius);

    // Scale the arrow shaft and position the cone at the end.
    const float arrow_scale = kArrowScale * radius;
    arrow_shaft_->SetScale(Vector3f(1, arrow_scale, 1));
    arrow_cone_->SetTranslation(Vector3f(0, arrow_scale, 0));

    // Set the current plane to match the last Plane in the ClippedModel if it
    // has any. Otherwise, use the XZ plane in local coordinates.
    const auto &primary = GetModel_(0);
    if (! primary.GetPlanes().empty()) {
        // Access the plane in the object coordinates of the unclipped
        // ClippedModel and transform it by the scale: the rotation and
        // translation of the ClippedModel are also applied to the ClipTool, so
        // they are not needed to convert the plane into the object coordinates
        // of the ClipTool.
        object_plane_ = TransformPlane(primary.GetPlanes().back(),
                                        ScaleMatrixH(primary.GetScale()));

        // Compensate for the mesh offset in the normal direction.
        const Vector3f offset_vec =
            primary.GetModelMatrix() * -primary.GetMeshOffset();
        object_plane_.distance -= Dot(offset_vec, object_plane_.normal);
    }
    else {
        object_plane_ = Plane(0, Vector3f::AxisY());
    }

    MatchPlane_();
}

void ClipTool::Impl_::Detach() {
    selection_.Clear();
}

SG::NodePtr ClipTool::Impl_::UpdateGripInfo(GripInfo &info,
                                            const Vector3f &guide_dir) {
    // If the direction is close to the arrow direction (either way), use the
    // translator.
    const Vector3f &arrow_dir = object_plane_.normal;
    if (AreDirectionsClose(guide_dir,  arrow_dir, TK::kMaxGripHoverDirAngle) ||
        AreDirectionsClose(guide_dir, -arrow_dir, TK::kMaxGripHoverDirAngle)) {
        info.widget = arrow_;
        return arrow_cone_;
    }
    else {
        info.widget = rotator_;
        return rotator_;
    }
}

void ClipTool::Impl_::MatchPlane_() {
    // Use the plane normal to compute the rotation. The untransformed arrow
    // direction is the +Y axis.
    const Rotationf rot =
        Rotationf::RotateInto(Vector3f::AxisY(), object_plane_.normal);
    rotator_->SetRotation(rot);
    arrow_and_plane_->SetRotation(rot);

    // Update the range of the slider based on the size of the Model and the
    // normal direction.
    UpdateTranslationRange_();

    // Use the distance of the plane as the Slider1DWidget value without
    // notifying.
    arrow_->GetValueChanged().EnableObserver(this, false);
    arrow_->SetValue(object_plane_.distance);
    arrow_->GetValueChanged().EnableObserver(this, true);

    // Position the plane at the correct distance.
    plane_->SetTranslation(object_plane_.distance * Vector3f::AxisY());
}

void ClipTool::Impl_::UpdateTranslationRange_() {
    // Compute the min/max signed distances of any mesh vertex along the
    // current plane's normal vector. This assumes that the Model's mesh is
    // centered on the origin, so that the center point is at a distance of
    // 0. Note that the mesh points need to be scaled by the ClippedModel's
    // scale to bring them into the object coordinates of the ClipTool.
    const auto     &model   = GetModel_(0);
    const Vector3f &scale   = model.GetScale();
    const auto     &mesh    = model.GetMesh();
    float          min_dist =  std::numeric_limits<float>::max();
    float          max_dist = -std::numeric_limits<float>::max();
    for (const Point3f &p: mesh.points) {
        const float dist = SignedDistance(ScalePoint(p, scale),
                                          object_plane_.normal);
        min_dist = std::min(min_dist, dist);
        max_dist = std::max(max_dist, dist);
    }

    // Set the range, making sure not to clip away all of the mesh. Restrict
    // the maximum only if this is the first clipping plane in the Model.
    const bool limit_max = model.GetPlanes().empty();
    arrow_->SetRange(min_dist + TK::kMinClippedSize,
                     max_dist - (limit_max ? TK::kMinClippedSize : 0));
}

void ClipTool::Impl_::RotatorActivated_(bool is_activation) {
    if (! is_activation) {
        // Update the current plane's normal from the arrow+plane rotation,
        // which includes snapping.
        const Rotationf &rot = arrow_and_plane_->GetRotation();
        object_plane_.normal = rot * Vector3f::AxisY();

        // Apply the rotation to the SphereWidget.
        rotator_->SetRotation(rot);

        UpdateTranslationRange_();
        arrow_->SetInactiveColor(arrow_inactive_color_);
    }
    UpdateRealTimeClipPlane_(is_activation);
}

void ClipTool::Impl_::Rotate_() {
    // Update the current plane with the current rotation in object and stage
    // coordinates.
    object_plane_.normal = rotator_->GetRotation() * Vector3f::AxisY();
    UpdateStagePlane_();

    // Snap the current plane's rotation if requested.
    bool snapped_to_target = false;
    int  snapped_dim       = -1;
    if (! context_.is_modified_mode)
        SnapRotation_(snapped_to_target, snapped_dim);

    arrow_->SetInactiveColor(
        snapped_to_target ? GetSnappedFeedbackColor() :
        snapped_dim >= 0  ? SG::ColorMap::SGetColorForDimension(snapped_dim) :
        arrow_inactive_color_);

    // Rotate the arrow and plane geometry to match the current plane.
    arrow_and_plane_->SetRotation(GetPlaneRotation_());

    UpdateRealTimeClipPlane_(true);
}

void ClipTool::Impl_::TranslatorActivated_(bool is_activation) {
    if (is_activation) {
        start_distance_ = object_plane_.distance;
        feedback_ = context_.feedback_manager->Activate<LinearFeedback>();
        context_.target_manager->StartSnapping();
    }
    else {
        context_.target_manager->EndSnapping();
        context_.feedback_manager->Deactivate(feedback_);
        feedback_.reset();
        arrow_->SetInactiveColor(arrow_inactive_color_);
    }

    // Hide the rotator sphere while translation is active.
    rotator_->SetEnabled(! is_activation);

    UpdateRealTimeClipPlane_(is_activation);
}

void ClipTool::Impl_::Translate_() {
    // Update the current plane with the current translation in object and
    // stage coordinates.
    const float distance = arrow_->GetValue();
    object_plane_.distance = distance;
    UpdateStagePlane_();

    // Snap the current plane's translation if requested.
    bool is_snapped = false;
    if (! context_.is_modified_mode)
        SnapTranslation_(is_snapped);

    const Color color = is_snapped ? GetSnappedFeedbackColor() :
        SG::ColorMap::SGetColor("WidgetActiveColor");
    arrow_->SetActiveColor(color);

    // Move the plane Widget to the correct distance.
    plane_->SetTranslation(object_plane_.distance * Vector3f::AxisY());

    UpdateTranslationFeedback_(color);
    UpdateRealTimeClipPlane_(true);
}

void ClipTool::Impl_::PlaneClicked_() {
    auto command = CreateCommand<ChangeClipCommand>();
    command->SetFromSelection(selection_);
    command->SetPlane(stage_plane_);
    context_.command_manager->AddAndDo(command);
}

void ClipTool::Impl_::SnapRotation_(bool &snapped_to_target, int &snapped_dim) {
    auto &tm = *context_.target_manager;

    ASSERT(selection_.HasAny());
    const SG::CoordConv cc(selection_.GetPrimary());

    // Try to snap to the point target (in stage coordinates) if it is active.
    Rotationf snap_rot;
    if (tm.SnapToDirection(stage_plane_.normal, snap_rot)) {
        stage_plane_.normal = tm.GetPointTarget().GetDirection();
        UpdateObjectPlane_();
        snapped_to_target = true;
    }

    // Otherwise, try to snap to any of the principal axes. Use stage axes if
    // is_axis_aligned is true; otherwise, use object axes.
    else {
        // Tests snapping dir to a principal axis.
        const auto snap_to_axis = [&](const Vector3f &dir,
                                      const Vector3f &axis, bool is_stage){
            if (tm.ShouldSnapDirections(dir, axis, snap_rot)) {
                if (is_stage) {
                    stage_plane_.normal = axis;
                    UpdateObjectPlane_();
                    return true;
                }
                else {
                    object_plane_.normal = axis;
                    UpdateStagePlane_();
                    return true;
                }
            }
            return false;
        };

        const bool is_stage = context_.is_axis_aligned;
        const Vector3f &dir =
            is_stage ? stage_plane_.normal : object_plane_.normal;
        for (int dim = 0; dim < 3; ++dim) {
            const Vector3f axis = GetAxis(dim);
            if (snap_to_axis(dir,  axis, is_stage) ||
                snap_to_axis(dir, -axis, is_stage)) {
                snapped_dim = dim;
                break;
            }
        }
    }
}

void ClipTool::Impl_::SnapTranslation_(bool &is_snapped) {
    // All snapping has to take place in stage coordinates to use the correct
    // snap tolerance.
    auto &tm = *context_.target_manager;

    // Updates stage_plane_ and returns true if stage_plane_ is close to the
    // given point.
    auto snap_to_pt = [&](const Point3f &pt){
        const float distance = stage_plane_.GetDistanceToPoint(pt);
        if (std::abs(distance) <= TK::kSnapPointTolerance) {
            stage_plane_.distance += distance;
            return true;
        }
        return false;
    };

    // First try snapping to the point target.
    if (tm.IsPointTargetVisible())
        is_snapped = snap_to_pt(tm.GetPointTarget().GetPosition());

    // If not, try snapping to the center of the Model. Assume the center is
    // the origin in object coordinates.
    if (! is_snapped) {
        const SG::CoordConv cc(selection_.GetPrimary());
        is_snapped = snap_to_pt(cc.ObjectToRoot(Point3f::Zero()));
    }

    if (is_snapped)
        UpdateObjectPlane_();
}

void ClipTool::Impl_::UpdateStagePlane_() {
    stage_plane_ = TransformPlane(object_plane_, root_node_.GetModelMatrix());
}

void ClipTool::Impl_::UpdateObjectPlane_() {
    object_plane_ = TransformPlane(
        stage_plane_, ion::math::Inverse(root_node_.GetModelMatrix()));
}

void ClipTool::Impl_::UpdateTranslationFeedback_(const Color &color) {
    // Need to use stage coordinates.
    const Matrix4f osm = root_node_.GetModelMatrix();

    const Vector3f motion_dir = osm * object_plane_.normal;
    const Point3f  start_pt   = osm * Point3f(start_distance_ *
                                              object_plane_.normal);
    const Point3f  end_pt     = osm * Point3f(object_plane_.distance *
                                              object_plane_.normal);

    // Need signed distance.
    float distance = ion::math::Distance(start_pt, end_pt);
    if (start_distance_ > object_plane_.distance)
        distance = -distance;

    feedback_->SetColor(color);
    feedback_->SpanLength(start_pt, motion_dir, distance);
}

void ClipTool::Impl_::UpdateRealTimeClipPlane_(bool enable) {
    context_.root_model->EnableClipping(enable, stage_plane_);
}

// ----------------------------------------------------------------------------
// ClipTool functions.
// ----------------------------------------------------------------------------

ClipTool::ClipTool() {
}

void ClipTool::UpdateGripInfo(GripInfo &info) {
    // Convert the controller guide direction into coordinates of the Tool.
    const Vector3f guide_dir = -GetRotation() * info.guide_direction;
    auto node = impl_->UpdateGripInfo(info, guide_dir);
    info.target_point = ToWorld(node, Point3f::Zero());
}

bool ClipTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<ClippedModel>(sel);
}

void ClipTool::Attach() {
    ASSERT(Util::IsA<ClippedModel>(GetModelAttachedTo()));

    if (! impl_)
        impl_.reset(new Impl_(GetContext(), *this));

    // Match the ClippedModel's transform and pass the size of the Model for
    // scaling. Note: no need to use is_axis_aligned here, since that affects
    // only snapping.
    impl_->Attach(GetSelection(), MatchModelAndGetSize(true));
}

void ClipTool::Detach() {
    impl_->Detach();
}
