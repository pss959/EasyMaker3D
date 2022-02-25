#include "Tools/RotationTool.h"

#include <limits>

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Dimensionality.h"
#include "Feedback/LinearFeedback.h"
#include "Managers/ColorManager.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Math/Types.h"
#include "SG/Search.h"
#include "SG/Torus.h"
#include "Util/Assert.h"
#include "Widgets/DiscWidget.h"
#include "Widgets/SphereWidget.h"

// ----------------------------------------------------------------------------
// RotationTool::Parts_ struct.
// ----------------------------------------------------------------------------

/// This struct stores all of the parts the RotationTool needs to operate.
struct RotationTool::Parts_ {
    /// DiscWidget to rotate around each principal axis.
    DiscWidgetPtr     axis_rotators[3];

    /// Torus shape used to represent each axis rotator ring.
    SG::TorusPtr      axis_rings[3];

    /// SphereWidget for free rotation.
    SphereWidgetPtr   free_rotator;

    /// Feedback display in 3 dimensions.
    // XXXX AngularFeedback feedback[3];
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
    // XXXX
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
        rotator->SetInactiveColor(ColorManager::GetColorForDimension(dim));
        parts_->axis_rotators[dim] = rotator;

        // Access the Torus shape so it can be modified.
        ASSERT(rotator->GetShapes().size() == 1U);
        auto torus = Util::CastToDerived<SG::Torus>(rotator->GetShapes()[0]);
        ASSERT(torus);
        parts_->axis_rings[dim] = torus;

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

    // The feedback is stored when activated.
}

void RotationTool::UpdateGeometry_() {
    ASSERT(parts_);
    model_size_ = MatchModelAndGetSize(false);

    // If axis-aligned, undo the tool rotation. Do this here rather than in
    // MatchModelAndGetSize() so that tighter bounds are used for the Model.
    if (GetContext().is_axis_aligned)
        SetRotation(Rotationf::Identity());

    // Determine the proper outer radius for the axis-rotation DiscWidget rings
    // based on the Model bounds, then use it to update the Torus fields.
    const float outer_radius = GetOuterRadius_();
    for (int dim = 0; dim < 3; ++dim )
        parts_->axis_rings[dim]->SetOuterRadius(outer_radius);

    // Update the scale of the free rotator.
    const float kSphereRadiusScale = .9f;
    parts_->free_rotator->SetUniformScale(kSphereRadiusScale * outer_radius);
}

void RotationTool::AxisRotatorActivated_(int dim, bool is_activation) {
    const auto &rotator = parts_->axis_rotators[dim];
    Dimensionality dims;
    dims.AddDimension(dim);

    if (is_activation) {
        start_rot_   = GetRotation();
        is_in_place_ = GetContext().is_alternate_mode;

        // Turn off all the other widgets.
        for (int i = 0; i < 3; ++i) {
            if (i != dim)
                parts_->axis_rotators[i]->SetEnabled(false);
        }
        parts_->free_rotator->SetEnabled(false);

        GetContext().target_manager->StartSnapping();

        // Turn on feedback for the dimension.
        EnableFeedback_(dims, true);

        // Enable the change callback.
        rotator->GetRotationChanged().EnableObserver(this, true);
    }
    else {
        // Disable the rotation change callback.
        rotator->GetRotationChanged().EnableObserver(this, false);

        // Turn off feedback.
        EnableFeedback_(dims, false);

        // Turn all the other widgets back on and put all the geometry in the
        // right places.
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
}

void RotationTool::AxisRotatorChanged_(int dim, const Anglef &angle) {
    const auto &context = GetContext();

    // If this is the first change, create the RotateCommand and start the
    // drag.
    CreateCommandIfNecessary_();

    // Try snapping to the target direction, getting a new angle if snapped.
    Anglef new_angle = angle;
#if XXXX
    bool isSnapped = SnapAxisRotation(model, dim, ref newAngle);
#endif
    bool is_snapped = false;  // XXXX

    // If not snapped, adjust by current precision.
    if (! is_snapped)
        new_angle = context.precision_manager->ApplyAngle(angle);

    // Compute the rotation and simulate execution of the command to update all
    // the Models.
    const Rotationf rot = Rotationf::FromAxisAndAngle(GetAxis(dim), new_angle);
    std::cerr << "XXXX rot = " << rot << "\n";
    command_->SetRotation(rot);
    context.command_manager->SimulateDo(command_);

    UpdateFeedback_(/* model, dim, newAngle, isSnapped */);
}

void RotationTool::CreateCommandIfNecessary_() {
    if (! command_) {
        command_ = CreateCommand<RotateCommand>("RotateCommand");
        command_->SetFromSelection(GetSelection());
        command_->SetIsInPlace(is_in_place_);
        command_->SetIsAxisAligned(GetContext().is_axis_aligned);
        GetDragStarted().Notify(*this);
    }
}

void RotationTool::FreeRotatorActivated_(bool is_activation) {
    // XXXX
}

void RotationTool::FreeRotatorChanged_(const Rotationf &rot) {
    // XXXX
}

void RotationTool::EnableFeedback_(const Dimensionality &dims, bool show) {
#if XXXX
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
#endif
}

void RotationTool::UpdateFeedback_() {
    // XXXX
}

float RotationTool::GetOuterRadius_() const {
    // Use just over half the diagonal length.
    const float kOuterRadiusScale = .51f;
    return kOuterRadiusScale * ion::math::Length(model_size_);
}
