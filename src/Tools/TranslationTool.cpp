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

void TranslationTool::Attach(const SelPath &path) {
    if (! parts_)
        FindParts_();

    const Model &model = *path.GetModel();

    // Rotate to match the Model if not aligning with stage axes.
    const bool is_aligned = GetContext().is_axis_aligned;
    SetRotation(is_aligned ? Rotationf::Identity() : model.GetRotation());


    // Move the Tool to the center of the Model in stage coordinates.
    const Matrix4f m = GetLocalToStageMatrix(path, false);
    SetTranslation(m * model.GetTranslation());

    // Determine the size to use for the sliders.
    Vector3f size = model.GetScaledBounds().GetSize();
    if (! is_aligned) {
        for (int i = 0; i < 3; ++i)
            size[i] *= m[i][i];
    }

    for (int i = 0; i < 3; ++i) {
        Parts_::DimParts &dp = parts_->dim_parts[i];
        dp.min_face->SetTranslation(Vector3f(-.5f * size[i], 0, 0));
        dp.max_face->SetTranslation(Vector3f( .5f * size[i], 0, 0));
        Vector3f scale = dp.stick->GetScale();
        scale[0] = size[i];
        dp.stick->SetScale(scale);
    }

    // XXXX Do something!!
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
    }
}

void TranslationTool::Detach() {
    // XXXX Do something!
}
