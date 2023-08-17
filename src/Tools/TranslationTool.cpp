#include "Tools/TranslationTool.h"

#include <ion/math/transformutils.h>

#include "Feedback/LinearFeedback.h"
#include "Items/SessionState.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/TargetManager.h"
#include "Math/Types.h"
#include "Models/Model.h"
#include "Place/PrecisionStore.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"
#include "Widgets/Slider1DWidget.h"

// ----------------------------------------------------------------------------
// TranslationTool::Parts_ struct.
// ----------------------------------------------------------------------------

/// This struct stores all of the parts the TranslationTool needs to operate.
struct TranslationTool::Parts_ {
    // Per-dimension parts.
    struct DimParts {
        Slider1DWidgetPtr slider;
        SG::NodePtr       min_face;
        SG::NodePtr       max_face;
        SG::NodePtr       stick;
    };
    DimParts          dim_parts[3];
    LinearFeedbackPtr feedback;
};

// ----------------------------------------------------------------------------
// TranslationTool functions.
// ----------------------------------------------------------------------------

TranslationTool::TranslationTool() {
}

void TranslationTool::CreationDone() {
    Tool::CreationDone();

    if (! IsTemplate())
        FindParts_();
}

void TranslationTool::UpdateGripInfo(GripInfo &info) {
    info.guide_type = GripGuideType::kBasic;

    // Convert the controller guide direction into coordinates of the Tool.
    const Vector3f guide_dir = -GetRotation() * info.guide_direction;

    // Use the controller orientation to get the best part to hover.
    bool is_opposite;
    const int index =
        GetBestAxis(guide_dir, TK::kMaxGripHoverDirAngle, is_opposite);
    if (index >= 0) {
        // Get the Face from the Slider1DWidget for the chosen index.
        const auto &slider = parts_->dim_parts[index].slider;
        auto face = SG::FindNodeUnderNode(*slider,
                                          is_opposite ? "MaxFace" : "MinFace");
        info.widget = slider;
        info.color  = GetFeedbackColor(index, false);

        // Set the target to the apex point on the face.
        const float z_offset = .5f * face->GetBounds().GetSize()[0];
        info.target_point = ToWorld(face, Point3f(0, 0, z_offset));
    }
    else {
        // Nothing was close.
        info.widget.reset();
    }
}

void TranslationTool::Attach() {
    ASSERT(parts_);
    UpdateGeometry_();
}

void TranslationTool::Detach() {
    // Nothing to do here.
}

void TranslationTool::FindParts_() {
    ASSERT(! parts_);
    parts_.reset(new Parts_);

    // Find all of the necessary parts.
    for (int dim = 0; dim < 3; ++dim) {
        Str dim_name = "X";
        dim_name[0] += dim;

        Parts_::DimParts &dp = parts_->dim_parts[dim];
        dp.slider = SG::FindTypedNodeUnderNode<Slider1DWidget>(
            *this, dim_name + "Slider");
        dp.min_face = SG::FindNodeUnderNode(*dp.slider, "MinFace");
        dp.max_face = SG::FindNodeUnderNode(*dp.slider, "MaxFace");
        dp.stick    = SG::FindNodeUnderNode(*dp.slider, "Stick");

        // Add observers to the slider.
        dp.slider->GetActivation().AddObserver(
            this, [&, dim](Widget &, bool is_act){
                SliderActivated_(dim, is_act); });

        dp.slider->GetValueChanged().AddObserver(
            this, [&, dim](Widget &, const float &val){
                SliderChanged_(dim, val); });
    }

    // The feedback is stored when activated.
}

void TranslationTool::UpdateGeometry_() {
    ASSERT(GetModelAttachedTo());
    const Model &model = *GetModelAttachedTo();

    // Rotate to match the Model if not aligning with stage axes.
    const bool is_aligned = IsAxisAligned();
    SetRotation(is_aligned ? Rotationf::Identity() : model.GetRotation());

    // Move the Tool to the center of the Model in stage coordinates.
    const Matrix4f lsm = GetStageCoordConv().GetLocalToRootMatrix();
    SetTranslation(lsm * model.GetTranslation());

    // Determine the size of the Model in stage coordinates. If aligned to
    // axes, use the axis-aligned box size.
    if (is_aligned) {
        const Matrix4f osm = GetStageCoordConv().GetObjectToRootMatrix();
        model_size_ = TransformBounds(model.GetBounds(), osm).GetSize();
    }
    else {
        model_size_ = model.GetScaledBounds().GetSize();
        for (int i = 0; i < 3; ++i)
            model_size_[i] *= lsm[i][i];
    }

    // Choose a good size for the min/max face handles and stick.
    const float kHandleSizeFraction = .1f;
    const float kMinHandleScale     = .2f;
    const float kMaxHandleScale     = .8f;
    const float kExtraStickLength   = .4f;
    const float handle_scale = ComputePartScale(
        model_size_, kHandleSizeFraction, kMinHandleScale, kMaxHandleScale);
    const float thickness_scale = .4f * handle_scale;

    for (int i = 0; i < 3; ++i) {
        auto &dp = parts_->dim_parts[i];
        const float sz = .5f * model_size_[i];

        // Scale and move the min/max faces.
        const Vector3f face_scale(handle_scale, handle_scale,
                                  2 * thickness_scale);
        dp.min_face->SetScale(face_scale);
        dp.max_face->SetScale(face_scale);
        dp.min_face->SetTranslation(Vector3f(-sz, 0, 0));
        dp.max_face->SetTranslation(Vector3f( sz, 0, 0));

        // Scale the stick.
        dp.stick->SetScale(Vector3f(model_size_[i] + kExtraStickLength,
                                    thickness_scale, thickness_scale));

        // Reset the slider to 0. Temporarily disable the observer so that it
        // does not try to update the tool.
        dp.slider->GetValueChanged().EnableObserver(this, false);
        dp.slider->SetValue(0);
        dp.slider->GetValueChanged().EnableObserver(this, true);
    }
}

void TranslationTool::SliderActivated_(int dim, bool is_activation) {
    if (is_activation) {
        // Save the starting information.
        start_value_ = GetSliderValue_(dim);

        // Hide all of the other sliders.
        for (int i = 0; i < 3; ++i)
            parts_->dim_parts[i].slider->SetEnabled(i == dim);

        GetContext().target_manager->StartSnapping();

        // Save the starting points of the translation in stage coordinates for
        // snapping to the point target.
        const Matrix4f lsm  = GetStageCoordConv().GetLocalToRootMatrix();
        const Point3f  pos  = Point3f(GetModelAttachedTo()->GetTranslation());
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
        if (command_)
            GetContext().feedback_manager->Deactivate(parts_->feedback);
        parts_->feedback.reset();

        // If there was any change due to a drag, execute the command to change
        // the transforms.
        if (command_) {
            if (command_->GetTranslation() != Vector3f::Zero())
                GetContext().command_manager->AddAndDo(command_);
            command_.reset();
        }
    }
}

void TranslationTool::SliderChanged_(int dim, const float &value) {
    // If this is the first change, create the TranslateCommand and start the
    // drag.
    if (! command_) {
        command_ = Command::CreateCommand<TranslateCommand>();
        command_->SetFromSelection(GetSelection());
        GetDragStarted().Notify(*this);

        // Activate the feedback.
        parts_->feedback =
            GetContext().feedback_manager->Activate<LinearFeedback>();
    }

    // Determine the change in value of the slider as a motion vector and
    // transform it into stage coordinates.
    const float new_value = GetSliderValue_(dim);
    const Vector3f axis = GetAxis(dim, new_value - start_value_);
    Vector3f motion =
        GetStageCoordConv().GetLocalToRootMatrix() * (GetRotation() * axis);

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
            motion *= GetContext().precision_store->Apply(length) / length;
        }
    }

    // Simulate execution of the command to update all the Models.
    command_->SetTranslation(motion);
    GetContext().command_manager->SimulateDo(command_);

    // Update the feedback using the motion vector.
    UpdateFeedback_(dim, motion, is_snapped);
}

void TranslationTool::UpdateFeedback_(int dim, const Vector3f &motion,
                                      bool is_snapped) {
    // Compute the vector and direction of motion in stage coordinates. This
    // has to be correct even when the motion vector has zero length.
    const Matrix4f osm        = GetStageCoordConv().GetObjectToRootMatrix();
    const Vector3f motion_dir = ion::math::Normalized(osm * GetAxis(dim));
    const float    sign       = ion::math::Dot(motion_dir, motion) < 0 ? -1 : 1;

    // Update the feedback object.
    parts_->feedback->SetColor(GetFeedbackColor(dim, is_snapped));
    parts_->feedback->SpanLength(start_stage_pos_, motion_dir,
                                 sign * ion::math::Length(motion));
}

float TranslationTool::GetSliderValue_(int dim) const {
    return parts_->dim_parts[dim].slider->GetValue();
}
