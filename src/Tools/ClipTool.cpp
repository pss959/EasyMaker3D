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
    Impl_(const Tool::Context &context, const SG::Node &root_node,
          const SG::CoordConv &stage_cc);

    void Attach(const Selection &sel, const Vector3f &model_size);
    void Detach();

    /// Implements UpdateGripInfo() for the ClipTool. Returns a Node that
    /// should be used to compute the target point.
    SG::NodePtr UpdateGripInfo(GripInfo &info);

  private:
    const Tool::Context &context_;

    /// The ClipTool as a Node, which is used for coordinate conversions.
    const SG::Node      &root_node_;

    /// SG::CoordConv used to convert to and from stage coordinates.
    const SG::CoordConv stage_cc_;

    /// This stores the current selection (should be all ClippedModels).
    Selection           selection_;

    /// Interactive PlaneWidget used to orient and position the clipping
    /// plane.
    PlaneWidgetPtr      plane_widget_;

    /// Plane (stage coordinates) at the start of widget interaction.
    Plane               start_plane_;

    /// Feedback showing translation distance in local coordinates.
    LinearFeedbackPtr   feedback_;

    /// Sets the min/max range for the translation slider based on the Model's
    /// mesh extents along the current plane's normal.
    void UpdateTranslationRange_();

    // Widget callbacks.
    void Activate_(bool is_activation);
    void PlaneChanged_(bool is_rotation);

    /// Updates feedback for plane translation.
    void UpdateTranslationFeedback_(const Color &color);

    /// Updates the state of the real-time clipping plane implemented in the
    /// Faceted shader.
    void UpdateRealTimeClipPlane_(bool enable);

    /// Returns the primary ClippedModel.
    ClippedModel & GetPrimary_() const {
        const auto primary = selection_.GetPrimary().GetModel();
        const auto &cm = Util::CastToDerived<ClippedModel>(primary);
        ASSERT(cm);
        return *cm;
    }
};

ClipTool::Impl_::Impl_(const Tool::Context &context, const SG::Node &root_node,
                       const SG::CoordConv &stage_cc) :
    context_(context),
    root_node_(root_node),
    stage_cc_(stage_cc) {

    plane_widget_ =
        SG::FindTypedNodeUnderNode<PlaneWidget>(root_node, "PlaneWidget");

    // Set up callbacks.
    const auto &rot = plane_widget_->GetRotator();
    const auto &tr  = plane_widget_->GetTranslator();

    plane_widget_->GetActivation().AddObserver(
        this, [&](Widget &, bool is_activation){ Activate_(is_activation); });
    plane_widget_->GetPlaneChanged().AddObserver(
        this, [&](bool is_rotation){ PlaneChanged_(is_rotation); });
}

void ClipTool::Impl_::Attach(const Selection &sel, const Vector3f &model_size) {
    using ion::math::Dot;
    using ion::math::Inverse;
    using ion::math::ScaleMatrixH;

    selection_ = sel;

    // Update the widget size based on the model size.
    const float radius = .5f * ion::math::Length(model_size);
    plane_widget_->SetSize(radius);


    // Match the Plane in the ClippedModel, which is stored in object
    // coordinates.
    const auto &primary = GetPrimary_();
    Plane object_plane = primary.GetPlane();

    // Compensate for the mesh offset in the normal direction.
    const Vector3f offset_vec =
        primary.GetRotation() * -primary.GetMeshOffset();
    object_plane.distance -= Dot(offset_vec, object_plane.normal);

    // Convert to stage coordinates.
    const Plane stage_plane = TransformPlane(object_plane,
                                             stage_cc_.GetObjectToRootMatrix());
    plane_widget_->SetPlane(stage_plane);

    // Update the range of the slider based on the size of the Model and the
    // normal direction.
    UpdateTranslationRange_();
}

void ClipTool::Impl_::Detach() {
    selection_.Clear();
}

SG::NodePtr ClipTool::Impl_::UpdateGripInfo(GripInfo &info) {
    // If the direction is close to the plane normal (in either direction), use
    // the translator.
    const Vector3f &normal = plane_widget_->GetPlane().normal;
    if (AreDirectionsClose(info.guide_direction,  normal,
                           TK::kMaxGripHoverDirAngle) ||
        AreDirectionsClose(info.guide_direction, -normal,
                           TK::kMaxGripHoverDirAngle)) {
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
    // 0. Convert the mesh points into stage coordinates so they can be
    // compared with the plane.
    const auto     &model   = GetPrimary_();
    const auto     &mesh    = model.GetMesh();
    const auto &normal = plane_widget_->GetPlane().normal;
    float          min_dist =  std::numeric_limits<float>::max();
    float          max_dist = -std::numeric_limits<float>::max();
    for (const Point3f &p: mesh.points) {
        const float dist = SignedDistance(stage_cc_.ObjectToRoot(p), normal);
        min_dist = std::min(min_dist, dist);
        max_dist = std::max(max_dist, dist);
    }

    // Set the translation range, making sure not to clip away all of the mesh
    // by restricting the minimum and maximum values.
    plane_widget_->SetTranslationRange(Range1f(min_dist + TK::kMinClippedSize,
                                               max_dist - TK::kMinClippedSize));
}

void ClipTool::Impl_::Activate_(bool is_activation) {
    if (is_activation) {
        start_plane_ = plane_widget_->GetPlane();
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
    const auto &current_plane = plane_widget_->GetPlane();
    const float distance = current_plane.distance - start_plane_.distance;

    feedback_->SetColor(color);
    feedback_->SpanLength(Point3f::Zero(), current_plane.normal, distance); // XXXX pos
}

void ClipTool::Impl_::UpdateRealTimeClipPlane_(bool enable) {
    context_.root_model->EnableClipping(enable, plane_widget_->GetPlane());
}

// ----------------------------------------------------------------------------
// ClipTool functions.
// ----------------------------------------------------------------------------

ClipTool::ClipTool() {
}

void ClipTool::UpdateGripInfo(GripInfo &info) {
    auto node = impl_->UpdateGripInfo(info);
    info.target_point = ToWorld(node, Point3f::Zero());
}

bool ClipTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<ClippedModel>(sel);
}

void ClipTool::Attach() {
    ASSERT(Util::IsA<ClippedModel>(GetModelAttachedTo()));

    if (! impl_)
        impl_.reset(new Impl_(GetContext(), *this, GetStageCoordConv()));

    // Match the ClippedModel's transform and pass the size of the Model for
    // scaling. Note: no need to use is_axis_aligned here, since that affects
    // only snapping.
    impl_->Attach(GetSelection(), MatchModelAndGetSize(true));
}

void ClipTool::Detach() {
    impl_->Detach();
}
