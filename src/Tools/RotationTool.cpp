#include "Tools/RotationTool.h"

#include <limits>

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Feedback/AngularFeedback.h"
#include "Items/SessionState.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/TargetManager.h"
#include "Math/Dimensionality.h"
#include "Math/Linear.h"
#include "Place/PrecisionStore.h"
#include "SG/Search.h"
#include "SG/Torus.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"
#include "Widgets/DiscWidget.h"
#include "Widgets/SphereWidget.h"

// ----------------------------------------------------------------------------
// RotationTool::Parts_ struct.
// ----------------------------------------------------------------------------

/// This struct stores all of the parts the RotationTool needs to operate.
struct RotationTool::Parts_ {
    /// DiscWidget to rotate around each principal axis.
    DiscWidgetPtr      axis_rotators[3];

    /// Torus shape used to represent each axis rotator ring.
    SG::TorusPtr       axis_rings[3];

    /// SphereWidget for free rotation.
    SphereWidgetPtr    free_rotator;

    /// Node displaying the sphere for free rotation.
    SG::NodePtr        free_sphere;

    /// Node showing axes during free rotation.
    SG::NodePtr        free_axes;

    /// Node shared by axes that is scaled to correct size.
    SG::NodePtr        free_axis;

    /// Feedback display in 3 dimensions.
    AngularFeedbackPtr feedback[3];
};

// ----------------------------------------------------------------------------
// RotationTool functions.
// ----------------------------------------------------------------------------

RotationTool::RotationTool() {
}

void RotationTool::CreationDone() {
    Tool::CreationDone();

    if (! IsTemplate())
        FindParts_();
}

void RotationTool::UpdateGripInfo(GripInfo &info) {
    info.guide_type = GripGuideType::kRotation;

    // Convert the controller guide direction into coordinates of the Tool.
    const Vector3f guide_dir = -GetRotation() * info.guide_direction;

    // Determine whether the controller direction is close to being aligned
    // with the axis of one of the axis rotators. If not, use the free rotator.
    bool is_opposite;
    const int index =
        GetBestAxis(guide_dir, TK::kMaxGripHoverDirAngle, is_opposite);
    if (index >= 0) {
        info.widget = parts_->axis_rotators[index];
        info.color  = GetFeedbackColor(index, false);
    }
    else {
        // Not close to any dimension. Use the free rotator.
        info.widget = parts_->free_rotator;
        info.color  = GetNeutralGripColor();
    }

    // Always point to the center of the widget. Nothing looks much better.
    info.target_point = ToWorld(info.widget, Point3f::Zero());
}

void RotationTool::Attach() {
    ASSERT(parts_);
    UpdateGeometry_();
}

void RotationTool::Detach() {
    // Nothing to do here.
}

void RotationTool::FindParts_() {
    ASSERT(! parts_);
    parts_.reset(new Parts_);

    // Initialize axis rotators.
    for (int dim = 0; dim < 3; ++dim ) {
        std::string name = ".AxisRotator";
        name[0] = 'X' + dim;
        auto rotator = SG::FindTypedNodeUnderNode<DiscWidget>(*this, name);
        parts_->axis_rotators[dim] = rotator;

        // Access the Torus shape so it can be modified.
        parts_->axis_rings[dim] =
            SG::FindTypedShapeInNode<SG::Torus>(*rotator, "Ring");

        // Add callbacks.
        rotator->GetActivation().AddObserver(
            this, [&, dim](Widget &, bool is_activation){
                AxisRotatorActivated_(dim, is_activation); });
        rotator->GetRotationChanged().AddObserver(
            this, [&, dim](Widget &, const Anglef &angle){
                AxisRotatorChanged_(dim, angle); });
        rotator->GetRotationChanged().EnableObserver(this, false);
    }

    // Initialize the free rotator.
    parts_->free_rotator =
        SG::FindTypedNodeUnderNode<SphereWidget>(*this, "FreeRotator");
    parts_->free_rotator->GetActivation().AddObserver(
            this, [&](Widget &, bool is_activation){
                FreeRotatorActivated_(is_activation); });
    parts_->free_rotator->GetRotationChanged().AddObserver(
            this, [&](Widget &, const Rotationf &rot){
                FreeRotatorChanged_(rot); });
    parts_->free_rotator->GetRotationChanged().EnableObserver(this, false);

    parts_->free_sphere =
        SG::FindNodeUnderNode(*parts_->free_rotator, "Sphere");

    parts_->free_axes = SG::FindNodeUnderNode(*this,              "Axes");
    parts_->free_axis = SG::FindNodeUnderNode(*parts_->free_axes, "Axis");

    // Turn off axes until free rotation starts.
    parts_->free_axes->SetEnabled(false);

    // The feedback is stored when activated.
}

void RotationTool::UpdateGeometry_() {
    ASSERT(parts_);
    model_size_ = MatchModelAndGetSize(false);

    // If axis-aligned, undo the tool rotation. Do this here rather than in
    // MatchModelAndGetSize() so that tighter bounds are used for the Model.
    if (IsAxisAligned())
        SetRotation(Rotationf::Identity());

    // Determine the proper outer radius for the axis-rotation DiscWidget rings
    // based on the Model bounds, then use it to update the Torus fields.
    // Adjust the inner radius as well.
    const float kInnerRadiusFraction = .1f;  // Relative to outer_radius.
    const float kMinInnerRadius      = .1f;
    const float kMaxInnerRadius      = .2f;
    const float outer_radius = GetOuterRadius_();
    const float inner_radius = Clamp(kInnerRadiusFraction * outer_radius,
                                     kMinInnerRadius, kMaxInnerRadius);
    for (int dim = 0; dim < 3; ++dim ) {
        parts_->axis_rings[dim]->SetOuterRadius(outer_radius);
        parts_->axis_rings[dim]->SetInnerRadius(inner_radius);
    }

    // Update the scale of the free rotator sphere.
    const float kSphereRadiusScale = .9f;
    const float free_radius = kSphereRadiusScale * outer_radius;
    parts_->free_sphere->SetUniformScale(free_radius);

    // And the axes used when doing free rotation.
    parts_->free_axis->SetScale(Vector3f(1, free_radius, 1));
}

void RotationTool::AxisRotatorActivated_(int dim, bool is_activation) {
    const auto &rotator = parts_->axis_rotators[dim];

    if (is_activation) {
        Activate_(Dimensionality(dim), dim);

        // Enable the change callback.
        rotator->GetRotationChanged().EnableObserver(this, true);
    }
    else {
        // Disable the rotation change callback.
        rotator->GetRotationChanged().EnableObserver(this, false);

        Deactivate_(Dimensionality(dim));
    }
}

void RotationTool::FreeRotatorActivated_(bool is_activation) {
    const auto &rotator = parts_->free_rotator;

    if (is_activation) {
        Activate_(Dimensionality("XYZ"), -1);

        // Hide the rotator sphere.
        parts_->free_sphere->SetFlagEnabled(SG::Node::Flag::kRender, false);

        // Show the axes.
        parts_->free_axes->SetEnabled(true);

        // Enable the change callback.
        rotator->GetRotationChanged().EnableObserver(this, true);
    }
    else {
        // Disable the rotation change callback.
        rotator->GetRotationChanged().EnableObserver(this, false);

        // Hide the axes.
        parts_->free_axes->SetEnabled(false);

        // Show the rotator sphere.
        parts_->free_sphere->SetFlagEnabled(SG::Node::Flag::kRender, true);

        Deactivate_(Dimensionality("XYZ"));
    }
}

void RotationTool::AxisRotatorChanged_(int dim, const Anglef &angle) {
    const auto &context = GetContext();

    // If this is the first change, create the RotateCommand and start the
    // drag.
    if (! command_)
        StartRotation_(Dimensionality(dim));

    // Try snapping to the target direction, getting a new angle if snapped.
    Rotationf rot = Rotationf::FromAxisAndAngle(GetAxis(dim), angle);

    // If snapping occurred, get the new angle (in the correct dimension) from
    // the snapped rotation. Otherwise, apply the current precision.
    Anglef new_angle = angle;
    const bool is_snapped = SnapRotation_(dim, rot) >= 0;
    if (is_snapped) {
        Anglef angles[3];
        rot.GetRollPitchYaw(&angles[2], &angles[0], &angles[1]);
        new_angle = angles[dim];
    }
    else {
        new_angle = context.precision_store->ApplyAngle(angle);
    }

    // Compute the rotation with the updated angle and simulate execution of
    // the command to update all the Models.
    rot = Rotationf::FromAxisAndAngle(GetAxis(dim), new_angle);
    command_->SetRotation(rot);
    context.command_manager->SimulateDo(command_);

    const float kTextOffset = 2;
    UpdateFeedback_(dim, new_angle, is_snapped, kTextOffset);
}

void RotationTool::FreeRotatorChanged_(const Rotationf &rot) {
    const auto &context = GetContext();

    // If this is the first change, create the RotateCommand and start the
    // drag.
    if (! command_)
        StartRotation_(Dimensionality("XYZ"));

    // Convert the rotation axis into RotationTool coordinates.
    Vector3f axis;
    Anglef   angle;
    rot.GetAxisAndAngle(&axis, &angle);
    Rotationf new_rot = Rotationf::FromAxisAndAngle(-start_rot_ * axis, angle);

    // Try snapping to the target direction, modifying the rotation if snapped.
    const int snapped_dim = SnapRotation_(-1, new_rot);

    // Simulate execution of the command to update all the Models.
    command_->SetRotation(new_rot);
    context.command_manager->SimulateDo(command_);

    // Rotate the axes by the current rotation.
    parts_->free_axes->SetRotation(new_rot);

    // Update feedback in all 3 dimensions.
    Anglef angles[3];
    new_rot.GetRollPitchYaw(&angles[2], &angles[0], &angles[1]);
    for (int dim = 0; dim < 3; ++dim) {
        // Offset the text by a different amount per dimension.
        const float kTextOffsetPerDim = 2;
        const float text_offset = kTextOffsetPerDim * dim;
        UpdateFeedback_(dim, angles[dim], dim == snapped_dim, text_offset);
    }
}

void RotationTool::Activate_(const Dimensionality &dims, int dim) {
    start_rot_   = GetRotation();
    is_in_place_ = GetContext().is_modified_mode;

    // Turn off all the other widgets.
    for (int i = 0; i < 3; ++i) {
        if (i != dim)
            parts_->axis_rotators[i]->SetEnabled(false);
    }
    if (dim >= 0)
        parts_->free_rotator->SetEnabled(false);

    GetContext().target_manager->StartSnapping();
}

void RotationTool::Deactivate_(const Dimensionality &dims) {
    // Turn off feedback.
    if (command_)
        EnableFeedback_(dims, false);

    // Reset the free rotator and axes rotation (to get the axes correct).
    parts_->free_rotator->SetRotation(Rotationf::Identity());
    parts_->free_axes->SetRotation(Rotationf::Identity());

    // Turn all other widgets on and put all the geometry in the right places.
    for (int i = 0; i < 3; ++i)
        parts_->axis_rotators[i]->SetEnabled(true);
    parts_->free_rotator->SetEnabled(true);
    UpdateGeometry_();

    // Invoke the DragEnded callbacks.
    GetDragEnded().Notify(*this);
    GetContext().target_manager->EndSnapping();

    // If there was any change due to a drag, execute the command to change
    // the transforms.
    if (command_) {
        if (command_->GetRotation() != Rotationf::Identity())
            GetContext().command_manager->AddAndDo(command_);
        command_.reset();
    }
}

void RotationTool::StartRotation_(const Dimensionality &dims) {
    ASSERT(! command_);
    command_ = Command::CreateCommand<RotateCommand>();
    command_->SetFromSelection(GetSelection());
    command_->SetIsInPlace(is_in_place_);
    command_->SetIsAxisAligned(IsAxisAligned());
    GetDragStarted().Notify(*this);

    // Turn on feedback for the dimension(s).
    EnableFeedback_(dims, true);
}

int RotationTool::SnapRotation_(int dim, Rotationf &rot) {
    const auto &context = GetContext();

    // Nothing to do if the point target is not visible.
    if (! context.target_manager->IsPointTargetVisible())
        return -1;

    // Get the current rotation (including rot).
    const bool is_aligned = IsAxisAligned();
    const Rotationf cur_rot = ComposeRotations_(start_rot_, rot, is_aligned);

    // Snap if any coordinate axis (except the one being rotated around) is now
    // close to the target direction.
    for (int i = 0; i < 3; ++i) {
        if (i == dim)
            continue;

        // Convert the rotated axis into stage coordinates.
        const Vector3f axis =
            GetStageCoordConv().LocalToRoot(cur_rot * GetAxis(i));

        Rotationf snapped_rot;
        if (context.target_manager->SnapToDirection(axis, snapped_rot)) {
            // Compute the rotation that, when composed with start_rot_,
            // will bring the axis to the target direction.
            rot = RotationDifference(start_rot_,
                                     ComposeRotations_(snapped_rot, cur_rot,
                                                       is_aligned));
            return i;
        }
    }
    return -1;
}

void RotationTool::EnableFeedback_(const Dimensionality &dims, bool show) {
    FeedbackManager &feedback_manager = *GetContext().feedback_manager;
    for (int dim = 0; dim < 3; ++dim) {
        if (dims.HasDimension(dim)) {
            auto &feedback = parts_->feedback[dim];
            if (show) {
                feedback = feedback_manager.Activate<AngularFeedback>();
            }
            else {
                feedback_manager.Deactivate<AngularFeedback>(feedback);
                feedback.reset();
            }
        }
    }
}

void RotationTool::UpdateFeedback_(int dim, const Anglef &angle,
                                   bool is_snapped, float text_up_offset) {
    // The feedback should be in the plane perpendicular to the rotation axis
    // (in stage coordinates).
    const Matrix4f osm = GetStageCoordConv().GetObjectToRootMatrix();
    const Vector3f axis = osm * GetAxis(dim);

    // Get the center of rotation in stage coordinates.
    const Point3f center = osm * Point3f::Zero();

    // Compute the radius of the Tool in stage coordinates and use that to
    // offset the feedback.
    const float offset = 2 + GetOuterRadius_();

    auto &feedback = parts_->feedback[dim];
    feedback->SetColor(GetFeedbackColor(dim, is_snapped));
    feedback->SubtendArc(center, offset, text_up_offset, axis,
                         CircleArc(Anglef(), angle));
}

float RotationTool::GetOuterRadius_() const {
    const float kRadiusScale = .51f;
    return kRadiusScale * ion::math::Length(model_size_);
}

Rotationf RotationTool::ComposeRotations_(const Rotationf &r0,
                                          const Rotationf &r1,
                                          bool is_axis_aligned) {
    return is_axis_aligned ? ComposeRotations(r0, r1) : ComposeRotations(r1, r0);
}
