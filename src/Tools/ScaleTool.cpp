#include "Tools/ScaleTool.h"

#include <limits>

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Base/Dimensionality.h"
#include "Base/Tuning.h"
#include "Feedback/LinearFeedback.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/PrecisionManager.h"
#include "Managers/TargetManager.h"
#include "Math/Types.h"
#include "Models/Model.h"
#include "SG/Search.h"
#include "Targets/EdgeTarget.h"
#include "Util/Assert.h"
#include "Widgets/ScaleWidget.h"
#include "Widgets/Slider1DWidget.h"

// ----------------------------------------------------------------------------
// ScaleTool::Scaler_ struct.
// ----------------------------------------------------------------------------

/// This struct stores information about a 1D, 2D, or 3D scaler.
struct ScaleTool::Scaler_ {
    ScaleWidgetPtr widget;       ///< ScaleWidget for the scaler.
    Vector3f       vector;       ///< Vector along which the scaler operates.
    Dimensionality dims;         ///< Which dimensions the scaler affects.
    SG::NodePtr    min_handle;   ///< Geometry for the minimum side handle.
    SG::NodePtr    max_handle;   ///< Geometry for the maximum side handle.
    SG::NodePtr    stick;        ///< Geometry for the stick joining handles.
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

    if (! IsTemplate()) {
        FindParts_();

        // Set up grip hovers.
        for (const auto &scaler: parts_->scalers)
            scaler_dirs_.push_back(scaler.vector);
    }
}

void ScaleTool::UpdateGripInfo(GripInfo &info) {
    // Convert the controller guide direction into coordinates of the Tool.
    const Vector3f guide_dir = -GetRotation() * info.guide_direction;

    // Use the controller orientation to get the best scaler to hover.
    bool is_opposite;
    const int index = GetBestDirIndex(scaler_dirs_, guide_dir,
                                      TK::kMaxGripHoverDirAngle, is_opposite);

    if (index >= 0) {
        const auto &scaler = parts_->scalers[index];
        info.widget = is_opposite ?
            scaler.widget->GetMaxSlider() : scaler.widget->GetMinSlider();
        const int dim = scaler.dims.GetIndex();
        if (dim >= 0)
            info.color = GetFeedbackColor(dim, false);
        info.target_point = ToWorld(info.widget, Point3f::Zero());
    }
    else {
        // Nothing was close.
        info.widget.reset();
    }
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

    // Set colors for 1D scalers.
    for (int dim = 0; dim < 3; ++dim) {
        const Color color = SG::ColorMap::SGetColorForDimension(dim);
        parts_->scalers[dim].widget->GetMinSlider()->SetInactiveColor(color);
        parts_->scalers[dim].widget->GetMaxSlider()->SetInactiveColor(color);
    }

    // The feedback is stored when activated.
}

void ScaleTool::InitScaler_(size_t index, const std::string &name,
                            const Vector3f &vec) {
    ASSERT(parts_);
    auto &scaler = parts_->scalers[index];

    scaler.widget     = SG::FindTypedNodeUnderNode<ScaleWidget>(*this, name);
    scaler.vector     = ion::math::Normalized(vec);
    scaler.min_handle = SG::FindNodeUnderNode(*scaler.widget, "MinSlider");
    scaler.max_handle = SG::FindNodeUnderNode(*scaler.widget, "MaxSlider");
    scaler.stick      = SG::FindNodeUnderNode(*scaler.widget, "Stick");

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

    // Rotate to match the Model. The scale tool always aligns with local axes.
    const Vector3f model_size = MatchModelAndGetSize(false);

    // Save the unscaled object bounds.
    ASSERT(GetModelAttachedTo());
    const Model &model = *GetModelAttachedTo();
    model_bounds_ = model.GetBounds();

    // These factors are used to scale vector lengths based on a Scaler_'s
    // Dimensionality. They are a little larger than they need to be so that
    // they are visible when attached to a Box.
    const float kLengthScale = 1.1f;
    const Vector3f dim_scales =
        kLengthScale * Vector3f(1, std::sqrt(2), std::sqrt(3));

    // Compute a reasonable scale for the handles based on the Model size.
    const float kHandleSizeFraction  = .25f;
    const float kMinHandleScale      = .2f;
    const float kMaxHandleScale      = .8f;
    const float k1DScalerExtraLength = 1;
    const float handle_scale = ComputePartScale(
        model_size, kHandleSizeFraction, kMinHandleScale, kMaxHandleScale);
    const float thickness_scale = .4f * handle_scale;

    for (auto &scaler: parts_->scalers) {
        // Each scaler is originally aligned with the +X axis. Determine the
        // correct direction vector and rotate to match it. For 2D and 3D
        // vectors, the direction is affected by the Model's size proportions.
        const Vector3f scaled_vec = model_size * scaler.vector;
        const Vector3f dir = .5f * ion::math::Normalized(scaled_vec);
        const Rotationf rot = Rotationf::RotateInto(Vector3f(1, 0, 0), dir);
        scaler.widget->SetRotation(rot);

        // Scale the handles based on the Model size.
        scaler.min_handle->SetUniformScale(handle_scale);
        scaler.max_handle->SetUniformScale(handle_scale);

        // Fix the orientations of the XY and YZ 2D slider handles.
        if (scaler.dims.GetCount() == 2 && scaler.dims.HasDimension(1)) {
            const int other_dim = scaler.dims.HasDimension(0) ? 0 : 2;
            const Rotationf orient =
                -rot * Rotationf::FromAxisAndAngle(GetAxis(other_dim),
                                                   Anglef::FromDegrees(90));
            scaler.min_handle->SetRotation(orient);
            scaler.max_handle->SetRotation(orient);
        }

        // Scale the stick thickness. Note that the length of the stick is set
        // by the ScaleWidget, so make sure to leave it as is.
        const Vector3f stick_scale = scaler.stick->GetScale();
        scaler.stick->SetScale(Vector3f(stick_scale[0],
                                        thickness_scale, thickness_scale));

        // Determine the min and max ScaleWidget values based on the size in
        // the direction of the vector. Note that 2D and 3D scalers use
        // dim_scales to compensate for their diagonal length.
        const int dim_count = scaler.dims.GetCount();
        const float dim_scale = dim_scales[dim_count - 1];
        const float half_size = .5f * ion::math::Length(scaled_vec) * dim_scale;
        // Add a little extra length to 1D scalers to make them stand out.
        const float extra_length = dim_count == 1 ? k1DScalerExtraLength : 0;
        scaler.widget->SetMinValue(-half_size - extra_length);
        scaler.widget->SetMaxValue( half_size + extra_length);
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

ScaleCommand::Mode ScaleTool::GetMode_(const Scaler_ &scaler) const {
    ScaleCommand::Mode mode;

    // For symmetric scales, check if the Model bounds lie on, intersect, or
    // are very close to the Y=0 plane. If so, use the kBaseSymmetric mode.
    if (scaler.widget->GetMode() == ScaleWidget::Mode::kSymmetric) {
        const Bounds bounds = TransformBounds(
            GetModelAttachedTo()->GetBounds(),
            GetStageCoordConv().GetObjectToRootMatrix());
        if (bounds.GetMinPoint()[1] <= TK::kCloseToStageForScaling)
            mode = ScaleCommand::Mode::kBaseSymmetric;
        else
            mode = ScaleCommand::Mode::kCenterSymmetric;
    }
    else {
        mode = ScaleCommand::Mode::kAsymmetric;
    }
    return mode;
}

void ScaleTool::ScalerChanged_(size_t index, bool is_max) {
    const Scaler_ &scaler = parts_->scalers[index];

    // If this is the first change, create the ScaleCommand and start the drag.
    if (! command_) {
        command_ = CreateCommand<ScaleCommand>();
        command_->SetFromSelection(GetSelection());
        command_->SetMode(GetMode_(scaler));
        GetDragStarted().Notify(*this);

        // Turn on feedback in all active dimensions.
        EnableFeedback_(scaler.dims, true);
    }

    // Compute the scale ratios in all affected dimensions and update the
    // ScaleCommand.
    Dimensionality snapped_dims;
    Vector3f ratios = ComputeRatios_(index, snapped_dims);
    if (command_->GetMode() == ScaleCommand::Mode::kAsymmetric) {
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
    /// \todo Use the dimension with the smallest difference.
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
