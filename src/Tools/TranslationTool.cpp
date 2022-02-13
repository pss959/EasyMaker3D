#include "Tools/TranslationTool.h"

#include <ion/math/transformutils.h>

#include "Feedback/LinearFeedback.h"
#include "Managers/ColorManager.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Math/Types.h"
#include "SG/Search.h"
#include "Util/Assert.h"
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
    // Use the controller orientation to get the best part to hover.
    std::vector<DirChoice> choices;
    choices.push_back(DirChoice("XSlider", Vector3f::AxisX()));
    choices.push_back(DirChoice("YSlider", Vector3f::AxisY()));
    choices.push_back(DirChoice("ZSlider", Vector3f::AxisZ()));

    const Anglef kMaxHoverDirAngle = Anglef::FromDegrees(20);
    bool is_opposite;
    const size_t index = GetBestDirChoiceSymmetric(
        choices, info.guide_direction, kMaxHoverDirAngle, is_opposite);

    if (index != ion::base::kInvalidIndex) {
        // Get the Face from the Slider1DWidget for the chosen index.
        auto slider = SG::FindTypedNodeUnderNode<Slider1DWidget>(
            *this, choices[index].name);
        auto face = SG::FindNodeUnderNode(*slider,
                                          is_opposite ? "MaxFace" : "MinFace");
        info.widget       = slider;
        info.target_point = ToWorld(face, Point3f::Zero());
        info.color        = GetFeedbackColor(index, false);
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
        std::string dim_name = "X";
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
    const bool is_aligned = GetContext().is_axis_aligned;
    SetRotation(is_aligned ? Rotationf::Identity() : model.GetRotation());

    // Move the Tool to the center of the Model in stage coordinates.
    const Matrix4f lsm = GetStageCoordConv().GetLocalToRootMatrix();
    SetTranslation(lsm * model.GetTranslation());

    // Determine the size to use for the sliders.
    model_size_ = model.GetScaledBounds().GetSize();
    if (! is_aligned) {
        for (int i = 0; i < 3; ++i)
            model_size_[i] *= lsm[i][i];
    }

    // Move the min/max faces and scale the stick.
    for (int i = 0; i < 3; ++i) {
        Parts_::DimParts &dp = parts_->dim_parts[i];
        const float sz = .5f * model_size_[i];
        dp.min_face->SetTranslation(Vector3f(-sz, 0, 0));
        dp.max_face->SetTranslation(Vector3f( sz, 0, 0));
        Vector3f scale = dp.stick->GetScale();
        scale[0] = model_size_[i];
        dp.stick->SetScale(scale);
        // Temporarily disable the observer so that it does not try to update
        // the tool.
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

        // Activate the feedback.
        parts_->feedback =
            GetContext().feedback_manager->Activate<LinearFeedback>();

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
        // This could be the end of a drag. If there was any motion, execute
        // the command to change the transforms.
        if (command_) {
            if (command_->GetTranslation() != Vector3f::Zero())
                GetContext().command_manager->AddAndDo(command_);
            command_.reset();
        }

        // Turn all the sliders back on and put all the geometry in the right
        // places.
        for (int i = 0; i < 3; ++i)
            parts_->dim_parts[i].slider->SetEnabled(true);
        UpdateGeometry_();

        // Deactivate the feedback.
        GetContext().feedback_manager->Deactivate(parts_->feedback);
        parts_->feedback.reset();

        // Invoke the DragEnded callbacks.
        GetDragEnded().Notify(*this);
        GetContext().target_manager->EndSnapping();
    }
}

void TranslationTool::SliderChanged_(int dim, const float &value) {
    // If this is the first change, create the TranslateCommand and start the
    // drag.
    if (! command_) {
        command_ = CreateCommand<TranslateCommand>("TranslateCommand");
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
    command_->SetTranslation(motion);
    GetContext().command_manager->SimulateDo(command_);

    // Update the feedback using the motion vector.
    UpdateFeedback_(dim, motion, is_snapped);
}

void TranslationTool::UpdateFeedback_(int dim, const Vector3f &motion,
                                      bool is_snapped) {
    // Get the starting and end points in stage coordinates. The motion vector
    // is already in stage coordinates.
    const Matrix4f lsm = GetStageCoordConv().GetLocalToRootMatrix();
    const Point3f  p0  = Point3f(lsm * GetTranslation());

    // Compute the direction of motion in stage coordinates. This has to be
    // correct even when the motion vector has zero length.
    const Vector3f motion_dir = ion::math::Normalized(lsm * GetAxis(dim));
    const float    sign       = ion::math::Dot(motion_dir, motion) < 0 ? -1 : 1;

    // Update the feedback object.
    parts_->feedback->SetColor(GetFeedbackColor(dim, is_snapped));
    parts_->feedback->SpanLength(p0, motion_dir,
                                 sign * ion::math::Length(motion));
}

float TranslationTool::GetSliderValue_(int dim) const {
    return parts_->dim_parts[dim].slider->GetValue();
}
