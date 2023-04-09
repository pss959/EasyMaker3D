#include "Tools/ClipTool.h"

// XXXX Check all of these
#include <algorithm>
#include <functional>
#include <limits>

#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Commands/ChangeClipCommand.h"
#include "Feedback/LinearFeedback.h"
#include "Items/SessionState.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/TargetManager.h"
#include "Math/Linear.h"
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
#include "Widgets/PlaneWidget.h"
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

    /// Interactive PlaneWidget used to orient and position the clipping
    /// plane.
    PlaneWidgetPtr      plane_widget_;

    /// Plane (stage coordinates) at the start of widget interaction.
    Plane               start_stage_plane_;

    /// Feedback showing translation distance in local coordinates.
    LinearFeedbackPtr   feedback_;

    /// Sets the min/max range for the translation slider based on the Model's
    /// mesh extents along the current plane's normal.
    void UpdateTranslationRange_();

    // Widget callbacks.
    void Activate_(bool is_activation, bool is_rotation);
    void PlaneChanged_(bool is_rotation);

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

    plane_widget_ =
        SG::FindTypedNodeUnderNode<PlaneWidget>(root_node, "PlaneWidget");

    // Set up callbacks.
    const auto &rot = plane_widget_->GetRotator();
    const auto &tr  = plane_widget_->GetTranslator();

    rot->GetActivation().AddObserver(this, [&](Widget &, bool is_activation){
        Activate_(is_activation, true); });
    tr->GetActivation().AddObserver(this, [&](Widget &, bool is_activation){
        Activate_(is_activation, false); });
    rot->GetRotationChanged().AddObserver(
        this, [&](Widget &, const Rotationf &){ PlaneChanged_(true); });
    tr->GetValueChanged().AddObserver(
        this, [&](Widget &, const float &){ PlaneChanged_(false); });
}

void ClipTool::Impl_::Attach(const Selection &sel, const Vector3f &model_size) {
    using ion::math::Dot;
    using ion::math::Inverse;
    using ion::math::ScaleMatrixH;

    selection_ = sel;

    // Update the widget size based on the model size.
    const float radius = .5f * ion::math::Length(model_size);
    plane_widget_->SetSize(radius);

    // Match the Plane in the ClippedModel. Access the plane in the object
    // coordinates of the unclipped ClippedModel and transform it by the scale:
    // the rotation and translation of the ClippedModel are also applied to the
    // ClipTool, so they are not needed to convert the plane into the object
    // coordinates of the ClipTool.
    const auto &primary = GetModel_(0);
    object_plane_ = TransformPlane(primary.GetPlane(),
                                   ScaleMatrixH(primary.GetScale()));

    // Compensate for the mesh offset in the normal direction.
    const Vector3f offset_vec =
        primary.GetModelMatrix() * -primary.GetMeshOffset();
    object_plane_.distance -= Dot(offset_vec, object_plane_.normal);

    stage_plane_ = TransformPlane(object_plane_, root_node_.GetModelMatrix());
    plane_widget_->SetPlane(stage_plane_);

    // Update the range of the slider based on the size of the Model and the
    // normal direction.
    UpdateTranslationRange_();
}

void ClipTool::Impl_::Detach() {
    selection_.Clear();
}

SG::NodePtr ClipTool::Impl_::UpdateGripInfo(GripInfo &info,
                                            const Vector3f &guide_dir) {
    // If the direction is close to the plane normal (in either direction), use
    // the translator.
    const Vector3f &normal = object_plane_.normal;
    if (AreDirectionsClose(guide_dir,  normal, TK::kMaxGripHoverDirAngle) ||
        AreDirectionsClose(guide_dir, -normal, TK::kMaxGripHoverDirAngle)) {
        info.widget = plane_widget_->GetTranslator();
    }
    else {
        info.widget = plane_widget_->GetRotator();
    }
    return info.widget;
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

    // Set the translation range, making sure not to clip away all of the mesh
    // by restricting the minimum and maximum values.
    plane_widget_->SetTranslationRange(Range1f(min_dist + TK::kMinClippedSize,
                                               max_dist - TK::kMinClippedSize));
}

void ClipTool::Impl_::Activate_(bool is_activation, bool is_rotation) {
    if (is_activation) {
        start_stage_plane_ = stage_plane_;
        feedback_ = context_.feedback_manager->Activate<LinearFeedback>();
        context_.target_manager->StartSnapping();
    }
    else {
        context_.target_manager->EndSnapping();
        context_.feedback_manager->Deactivate(feedback_);
        feedback_.reset();
    }

    UpdateRealTimeClipPlane_(is_activation);
}

void ClipTool::Impl_::PlaneChanged_(bool is_rotation) {
    UpdateRealTimeClipPlane_(true);
    UpdateTranslationRange_();
    if (! is_rotation)
        UpdateTranslationFeedback_(Color(1, 0, 0, 1));  // XXXX Color
}

void ClipTool::Impl_::UpdateTranslationFeedback_(const Color &color) {
    // Need signed distance in stage coordinates.
    const float distance = stage_plane_.distance - start_stage_plane_.distance;

    feedback_->SetColor(color);
    feedback_->SpanLength(Point3f::Zero(), stage_plane_.normal, distance); // XXXX pos
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
