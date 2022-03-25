#include "Tools/ScaleTool.h"

#include <limits>

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Dimensionality.h"
#include "Feedback/LinearFeedback.h"
#include "Managers/ColorManager.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/PrecisionManager.h"
#include "Managers/TargetManager.h"
#include "Math/Types.h"
#include "SG/Search.h"
#include "Targets/EdgeTarget.h"
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
    /// for a total of 13.
    Scaler_           scalers[13];

    /// Feedback display in 3 dimensions.
    LinearFeedbackPtr feedback[3];
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

    // Set up dims: add each dimension that is not 0 in the vector.
    for (int i = 0; i < 3; ++i)
        if (vec[i] != 0)
            scaler.dims.AddDimension(i);

    // Add callbacks.
    scaler.widget->GetActivation().AddObserver(
        this, [&, index](Widget &, bool is_activation){
            ScalerActivated_(index, is_activation); });
    scaler.widget->GetScaleChanged().AddObserver(
        this, [&, index](Widget &, bool is_max){
            ScalerChanged_(index, is_max); });
    scaler.widget->GetScaleChanged().EnableObserver(this, false);
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

void ScaleTool::ScalerActivated_(size_t index, bool is_activation) {
    const Scaler_ &scaler = parts_->scalers[index];

    if (is_activation) {
        // Turn off all the other scalers.
        for (size_t i = 0; i < 13U; ++i) {
            if (i != index)
                parts_->scalers[i].widget->SetEnabled(false);
        }

        // Save the current Model size. Note that this is just the scaled
        // object bounds for the Model - there is no need to take any higher-up
        // scaling into account because those would not be in effect if this
        // Model is being edited.
        const Model &model = *GetModelAttachedTo();
        start_model_size_ = model.GetScaledBounds().GetSize();

        // Save the starting length of the scaler to make it easier to compute
        // ratios.
        start_length_ = scaler.widget->GetLength();

        GetContext().target_manager->StartSnapping();

        // Enable the scale change callback.
        scaler.widget->GetScaleChanged().EnableObserver(this, true);
    }
    else {
        // Disable the scale change callback.
        scaler.widget->GetScaleChanged().EnableObserver(this, false);

        // Turn off feedback.
        if (command_)
            EnableFeedback_(scaler.dims, false);

        // Turn all the other scalers back on and put all the geometry in the
        // right places.
        for (int i = 0; i < 13; ++i)
            parts_->scalers[i].widget->SetEnabled(true);
        UpdateGeometry_();

        // Invoke the DragEnded callbacks.
        GetDragEnded().Notify(*this);
        GetContext().target_manager->EndSnapping();

        // If there was any change due to a drag, execute the command to change
        // the transforms.
        if (command_) {
            if (command_->GetRatios() != Vector3f(1, 1, 1))
                GetContext().command_manager->AddAndDo(command_);
            command_.reset();
        }
    }
}

void ScaleTool::ScalerChanged_(size_t index, bool is_max) {
    const Scaler_ &scaler = parts_->scalers[index];

    // If this is the first change, create the ScaleCommand and start the drag.
    if (! command_) {
        command_ = CreateCommand<ScaleCommand>();
        command_->SetFromSelection(GetSelection());
        command_->SetIsSymmetric(scaler.widget->GetMode() ==
                                 ScaleWidget::Mode::kSymmetric);
        GetDragStarted().Notify(*this);

        // Turn on feedback in all active dimensions.
        EnableFeedback_(scaler.dims, true);
    }

    // Compute the scale ratios in all affected dimensions and update the
    // ScaleCommand.
    Dimensionality snapped_dims;
    Vector3f ratios = ComputeRatios_(index, snapped_dims);
    if (! command_->IsSymmetric()) {
        // For asymmetric scales, set the ratio signs to indicate which side is
        // fixed.
        const Vector3f sign_vec = is_max ? scaler.vector : -scaler.vector;
        for (int dim = 0; dim < 3; ++dim)
            if (sign_vec[dim] < 0)
                ratios[dim] = -ratios[dim];
    }
    command_->SetRatios(ratios);

    // Simulate execution of the command to update all the Models.
    GetContext().command_manager->SimulateDo(command_);

    UpdateFeedback_(scaler.vector, scaler.dims, snapped_dims, is_max);
}

Vector3f ScaleTool::ComputeRatios_(size_t index, Dimensionality &snapped_dims) {
    // Note that this code ignores local rotation, which does not affect the
    // length of the vector between the points unless a shear is introduced in
    // some Transform above this. In that case, the sheared size is not really
    // a useful value anyway, so who knows what is better to do?

    // Compute the change in length of the scaler and apply that to the
    // scaler's direction vector.
    const Scaler_ &scaler = parts_->scalers[index];
    float ratio = scaler.widget->GetLength() / start_length_;

    // Compute the new Model size based on the ratio.
    Vector3f new_size = start_model_size_;
    for (int dim = 0; dim < 3; ++dim)
        if (scaler.dims.HasDimension(dim))
            new_size[dim] *= ratio;

    // Try snapping to the target edge length in all modified dimensions.  If
    // any of them snapped, use the first one.
    // TODO: Use the dimension with the smallest difference.
    TargetManager &target_manager = *GetContext().target_manager;
    snapped_dims = target_manager.SnapToLength(scaler.dims, new_size);
    if (snapped_dims.GetCount() > 0) {
        const int dim = snapped_dims.HasDimension(0) ? 0 :
            snapped_dims.HasDimension(1) ? 1 : 2;
        // Use the ratio in the snapped dimensions for all 3.
        const float snap_length = target_manager.GetEdgeTarget().GetLength();
        ratio = snap_length / start_model_size_[dim];
    }
    else {
        // Otherwise, apply the current precision. Compute the absolute
        // difference in sizes with and without precision, then find the active
        // dimension with the smallest difference.
        const Vector3f prec =
            GetContext().precision_manager->ApplyPositive(new_size);
        const Vector3f abs_diff(std::abs(new_size[0] - prec[0]),
                                std::abs(new_size[1] - prec[1]),
                                std::abs(new_size[2] - prec[2]));
        float smallest = std::numeric_limits<float>::max();
        int best_dim = -1;
        for (int dim = 0; dim < 3; ++dim) {
            if (scaler.dims.HasDimension(dim) && abs_diff[dim] < smallest) {
                best_dim = dim;
                smallest = abs_diff[dim];
            }
        }
        ASSERT(best_dim >= 0);

        // Use the ratio in that dimension.
        ratio = prec[best_dim] / start_model_size_[best_dim];
    }

    // Set the ratios in active dimensions.
    Vector3f ratios(1, 1, 1);
    for (int dim = 0; dim < 3; ++dim)
        if (scaler.dims.HasDimension(dim))
            ratios[dim] = ratio;

    return ratios;
}

void ScaleTool::EnableFeedback_(const Dimensionality &dims, bool show) {
    FeedbackManager &feedback_manager = *GetContext().feedback_manager;
    for (int dim = 0; dim < 3; ++dim) {
        if (dims.HasDimension(dim)) {
            auto &feedback = parts_->feedback[dim];
            if (show) {
                feedback = feedback_manager.Activate<LinearFeedback>();
            }
            else {
                feedback_manager.Deactivate<LinearFeedback>(feedback);
                feedback.reset();
            }
        }
    }
}

void ScaleTool::UpdateFeedback_(const Vector3f &scaler_vec,
                                const Dimensionality &scaler_dims,
                                const Dimensionality &snapped_dims,
                                bool is_max) {
    const Matrix4f osm = GetStageCoordConv().GetObjectToRootMatrix();
    for (int dim = 0; dim < 3; ++dim) {
        if (! scaler_dims.HasDimension(dim))
            continue;

        // Find the stage-space points in the centers of the scaled sides.
        const Point3f p0 = osm * model_bounds_.GetFaceCenter(
            Bounds::GetFace(dim, false));
        const Point3f p1 = osm * model_bounds_.GetFaceCenter(
            Bounds::GetFace(dim, true));

        auto &feedback = parts_->feedback[dim];
        ASSERT(feedback);
        feedback->SetColor(
            GetFeedbackColor(dim, snapped_dims.HasDimension(dim)));

        // Determine which point to use first, which affects feedback text
        // placement.
        if ((scaler_vec[dim] > 0) == is_max)
            feedback->SpanPoints(p0, p1);
        else
            feedback->SpanPoints(p1, p0);
    }
}
