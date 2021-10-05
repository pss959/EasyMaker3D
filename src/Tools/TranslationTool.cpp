#include "Tools/TranslationTool.h"

#include <ion/math/transformutils.h>

#include "Assert.h"
#include "Math/Types.h"
#include "SG/Search.h"
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
    DimParts dim_parts[3];
};

// ----------------------------------------------------------------------------
// TranslationTool functions.
// ----------------------------------------------------------------------------

TranslationTool::TranslationTool() {
}

void TranslationTool::Attach() {
    if (! parts_)
        FindParts_();
    UpdateGeometry_();
}

void TranslationTool::Detach() {
    // XXXX Do something!
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
            this, std::bind(&TranslationTool::SliderActivated_,
                            this, dim, std::placeholders::_1,
                            std::placeholders::_2));
        dp.slider->GetValueChanged().AddObserver(
            this, std::bind(&TranslationTool::SliderChanged_,
                            this, dim, std::placeholders::_1,
                            std::placeholders::_2));
    }
}

void TranslationTool::UpdateGeometry_() {
    ASSERT(GetPrimaryModel());
    const Model &model = *GetPrimaryModel();

    // Rotate to match the Model if not aligning with stage axes.
    const bool is_aligned = GetContext().is_axis_aligned;
    SetRotation(is_aligned ? Rotationf::Identity() : model.GetRotation());

    // Move the Tool to the center of the Model in stage coordinates.
    const Matrix4f lsm = GetLocalToStageMatrix(false);
    SetTranslation(lsm * model.GetTranslation());

    // Determine the size to use for the sliders.
    model_size_ = model.GetScaledBounds().GetSize();
    if (! is_aligned) {
        for (int i = 0; i < 3; ++i)
            model_size_[i] *= lsm[i][i];
    }

    for (int i = 0; i < 3; ++i) {
        Parts_::DimParts &dp = parts_->dim_parts[i];
        const float sz = .5f * model_size_[i];
        dp.min_face->SetTranslation(Vector3f(-sz, 0, 0));
        dp.max_face->SetTranslation(Vector3f( sz, 0, 0));
        Vector3f scale = dp.stick->GetScale();
        scale[0] = model_size_[i];
        dp.stick->SetScale(scale);
    }
}

void TranslationTool::SliderActivated_(int dim, Widget &widget,
                                       bool is_activation) {
    if (is_activation) {
        // Save the starting information.
        parts_->dim_parts[dim].slider->GetValue();

        // Hide all of the other sliders.
        for (int i = 0; i < 3; ++i)
            parts_->dim_parts[i].slider->SetEnabled(
                SG::Node::Flag::kTraversal, i == dim);

        // Activate the feedback.
        // XXXX feedback_ = ActivateFeedback<LinearFeedback>();

        // XXXX GetContext().target_manager->StartSnapping();

        // Save the starting points of the translation in stage coordinates for
        // snapping to the point target.
        const Matrix4f lsm  = GetLocalToStageMatrix(false);
        const Vector3f pos  = GetPrimaryModel()->GetTranslation();
        const Vector3f svec = GetAxis(dim, .5f * model_size_[dim]);
        start_stage_min_ = lsm * (pos - svec);
        start_stage_pos_ = lsm * pos;
        start_stage_max_ = lsm * (pos + svec);
    }
    else {
        // This could be the end of a drag. If there was any motion, execute
        // the command to change the transforms.
#if XXXX
        if (command_) {
            if (command_.GetTranslation() != Vector3f::Zero())
                GetContext().command_manager.AddAndDo(command_);
            command_.reset();
        }
#endif
        // Turn all the sliders back on and put all the geometry in the right
        // places.
        for (int i = 0; i < 3; ++i)
            parts_->dim_parts[i].slider->SetEnabled(
                SG::Node::Flag::kTraversal, true);
        UpdateGeometry_();

        // Deactivate the feedback.
        // XXXX DeactivateFeedback<LinearFeedback>(feedback_);
        // XXXX feedback_ = null;

        // Invoke the DragEnded callbacks.
        GetDragEnded().Notify(*this);
        // XXXX GetContext().target_manager->EndSnapping();
    }
}

void TranslationTool::SliderChanged_(int dim, Widget &widget,
                                     const float &value) {
    std::cerr << "XXXX SliderChanged_ dim=" << dim
              << " value=" << value << "\n";
}
