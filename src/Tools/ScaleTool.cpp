#include "Tools/ScaleTool.h"

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Dimensionality.h"
#include "Feedback/LinearFeedback.h"
#include "Managers/ColorManager.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Math/Types.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Widgets/ScaleWidget.h"

// ----------------------------------------------------------------------------
// ScaleTool::Scaler_ struct.
// ----------------------------------------------------------------------------

/// This struct stores information about a 1D, 2D, or 3D scaler.
struct ScaleTool::Scaler_ {
    ScaleWidgetPtr widget;  ///< ScaleWidget for the scaler.
    Vector3f       vector;  ///< Vector along which the scaler operates.
    Dimensionality dims;    ///< Which dimensions the scaler affects.
};

// ----------------------------------------------------------------------------
// ScaleTool::Parts_ struct.
// ----------------------------------------------------------------------------

/// This struct stores all of the parts the ScaleTool needs to operate.
struct ScaleTool::Parts_ {
    /// All Scalers_.  There are 3 1D scalers, 6 2D scalers, and 4 3D scalers
    // for a total of 13.
    Scaler_ scalers[13];
};

// ----------------------------------------------------------------------------
// ScaleTool functions.
// ----------------------------------------------------------------------------

ScaleTool::ScaleTool() {
}

void ScaleTool::CreationDone() {
    Tool::CreationDone();

    if (! IsTemplate())
        FindParts_();
}

void ScaleTool::UpdateGripInfo(GripInfo &info) {
    // XXXX
}

void ScaleTool::Attach() {
    ASSERT(parts_);
    UpdateGeometry_();
}

void ScaleTool::Detach() {
    // Nothing to do here.
}

void ScaleTool::FindParts_() {
    ASSERT(! parts_);
    parts_.reset(new Parts_);

    // Initialize 1D scalers.
    InitScaler_(0,  "XScaler", Vector3f(1, 0, 0));
    InitScaler_(1,  "YScaler", Vector3f(0, 1, 0));
    InitScaler_(2,  "ZScaler", Vector3f(0, 0, 1));

    // Initialize 2D scalers.
    InitScaler_(3,  "XY1Scaler", Vector3f( 1,  1, 0));
    InitScaler_(4,  "XY2Scaler", Vector3f(-1,  1, 0));
    InitScaler_(5,  "XZ1Scaler", Vector3f( 1,  0, 1));
    InitScaler_(6,  "XZ2Scaler", Vector3f(-1,  0, 1));
    InitScaler_(7,  "YZ1Scaler", Vector3f( 0,  1, 1));
    InitScaler_(8,  "YZ2Scaler", Vector3f( 0, -1, 1));

    // Initialize 3D scalers.
    InitScaler_(9,  "XYZ1Scaler", Vector3f(1,  -1, -1));
    InitScaler_(10, "XYZ2Scaler", Vector3f(1,  -1,  1));
    InitScaler_(11, "XYZ3Scaler", Vector3f(1,   1, -1));
    InitScaler_(12, "XYZ4Scaler", Vector3f(1,   1,  1));

    // The feedback is stored when activated.
}

void ScaleTool::InitScaler_(size_t index, const std::string &name,
                            const Vector3f &vec) {
    ASSERT(parts_);
    auto &scaler = parts_->scalers[index];

    scaler.widget = SG::FindTypedNodeUnderNode<ScaleWidget>(*this, name);
    scaler.vector = ion::math::Normalized(vec);

    // Add each dimension that is not 0 in the vector.
    for (int i = 0; i < 3; ++i)
        if (vec[i] != 0)
            scaler.dims.AddDimension(i);
}

void ScaleTool::UpdateGeometry_() {
    ASSERT(parts_);
    const Vector3f size = MatchModelAndGetSize(false);

    // Save the unscaled object bounds.
    ASSERT(GetModelAttachedTo());
    const Model &model = *GetModelAttachedTo();
    model_bounds_ = model.GetBounds();

    // These factors are used to scale vector lengths based on a Scaler_'s
    // Dimensionality. Note that 1D scalers are relatively larger to make them
    // easier to access.
    const Vector3f dim_scales(1.4f, std::sqrt(2), std::sqrt(3));

    for (auto &scaler: parts_->scalers) {
        // Each scaler is originally aligned with the +X axis. Determine the
        // correct direction vector and rotate to match it. For 2D and 3D
        // vectors, the direction is affected by the scale.
        const Vector3f scaled_vec = size * scaler.vector;
        const Vector3f dir = .5f * ion::math::Normalized(scaled_vec);
        const Rotationf rot = Rotationf::RotateInto(Vector3f(1, 0, 0), dir);
        scaler.widget->SetRotation(rot);

        // Inversely rotate the widget handles so they remain aligned with the
        // local axes of the Model.
        auto min = SG::FindNodeUnderNode(*scaler.widget, "MinSlider");
        auto max = SG::FindNodeUnderNode(*scaler.widget, "MaxSlider");
        min->SetRotation(-rot);
        max->SetRotation(-rot);

        // Determine the min and max ScaleWidget values based on the size in
        // the direction of the vector. Note that 2D and 3D scalers have to
        // compensate for their diagonal length.
        const float dim_scale = dim_scales[scaler.dims.GetCount() - 1];
        const float half_size = .5f * ion::math::Length(scaled_vec) * dim_scale;
        scaler.widget->SetMinValue(-half_size);
        scaler.widget->SetMaxValue( half_size);
    }
}

#if XXXX
void ScaleTool::SliderActivated_(int dim, bool is_activation) {
    if (is_activation) {
        // Save the starting information.
        start_value_ = GetSliderValue_(dim);

        // Hide all of the other sliders.
        for (int i = 0; i < 3; ++i)
            parts_->dim_parts[i].slider->SetEnabled(i == dim);

        // Activate the feedback.
        parts_->feedback =
            GetContext().feedback_manager->Activate<LinearFeedback>();

        GetContext().target_manager->StartSnapping();

        // Save the starting points of the scale in stage coordinates for
        // snapping to the point target.
        const Matrix4f lsm  = GetStageCoordConv().GetLocalToRootMatrix();
        const Point3f  pos  = Point3f(GetModelAttachedTo()->GetScale());
        const Vector3f svec = GetAxis(dim, .5f * model_size_[dim]);
        start_stage_min_ = lsm * (pos - svec);
        start_stage_pos_ = lsm * pos;
        start_stage_max_ = lsm * (pos + svec);
    }
    else {
        // Turn all the sliders back on and put all the geometry in the right
        // places.
        for (int i = 0; i < 3; ++i)
            parts_->dim_parts[i].slider->SetEnabled(true);
        UpdateGeometry_();

        // Invoke the DragEnded callbacks.
        GetDragEnded().Notify(*this);
        GetContext().target_manager->EndSnapping();

        // Deactivate the feedback.
        GetContext().feedback_manager->Deactivate(parts_->feedback);
        parts_->feedback.reset();

        // If there was any change due to a drag, execute the command to change
        // the transforms.
        if (command_) {
            if (command_->GetScale() != Vector3f::Zero())
                GetContext().command_manager->AddAndDo(command_);
            command_.reset();
        }
    }
}

void ScaleTool::SliderChanged_(int dim, const float &value) {
    // If this is the first change, create the ScaleCommand and start the
    // drag.
    if (! command_) {
        command_ = CreateCommand<ScaleCommand>("ScaleCommand");
        command_->SetFromSelection(GetSelection());
        GetDragStarted().Notify(*this);
    }

    // Determine the change in value of the slider as a motion vector and
    // transform it into stage coordinates.
    const float new_value = GetSliderValue_(dim);
    const Vector3f axis = GetAxis(dim, new_value - start_value_);
    Vector3f motion = GetStageCoordConv().GetLocalToRootMatrix() * axis;

    // Try snapping the bounds min, center, and max in the direction of motion
    // to the point target. If nothing snaps, adjust by the current precision.
    bool is_snapped = false;

    const float length = ion::math::Length(motion);
    if (length > 0) {
        auto &target_manager = *GetContext().target_manager;
        if (target_manager.SnapToPoint(start_stage_pos_, motion) ||
            target_manager.SnapToPoint(start_stage_min_, motion) ||
            target_manager.SnapToPoint(start_stage_max_, motion)) {
            is_snapped = true;
        }
        else {
            motion *= GetContext().precision_manager->Apply(length) / length;
        }
    }

    // Simulate execution of the command to update all the Models.
    command_->SetScale(motion);
    GetContext().command_manager->SimulateDo(command_);

    // Update the feedback using the motion vector.
    UpdateFeedback_(dim, motion, is_snapped);
}

void ScaleTool::UpdateFeedback_(int dim, const Vector3f &motion,
                                      bool is_snapped) {
    // Get the starting and end points in stage coordinates. The motion vector
    // is already in stage coordinates.
    const Matrix4f lsm = GetStageCoordConv().GetLocalToRootMatrix();
    const Point3f  p0  = Point3f(lsm * GetScale());

    // Compute the direction of motion in stage coordinates. This has to be
    // correct even when the motion vector has zero length.
    const Vector3f motion_dir = ion::math::Normalized(lsm * GetAxis(dim));
    const float    sign       = ion::math::Dot(motion_dir, motion) < 0 ? -1 : 1;

    // Update the feedback object.
    parts_->feedback->SetColor(GetFeedbackColor(dim, is_snapped));
    parts_->feedback->SpanLength(p0, motion_dir,
                                 sign * ion::math::Length(motion));
}

float ScaleTool::GetSliderValue_(int dim) const {
    return parts_->dim_parts[dim].slider->GetValue();
}
#endif
