#include "Tools/TwistTool.h"

#include <ion/math/vectorutils.h>

#include "Models/TwistedModel.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Widgets/DiscWidget.h"
#include "Widgets/Slider2DWidget.h"
#include "Widgets/SphereWidget.h"

void TwistTool::CreationDone() {
    Tool::CreationDone();

    if (! IsTemplate()) {
        // Find all of the parts.
        twister_ = SG::FindTypedNodeUnderNode<DiscWidget>(*this, "Twister");
        axis_rotator_ = SG::FindTypedNodeUnderNode<SphereWidget>(
            *this, "AxisRotator");
        center_translator_ = SG::FindTypedNodeUnderNode<Slider2DWidget>(
            *this, "CenterTranslator");

        center_translator_->SetRange(Vector2f(-100, -100), Vector2f(100, 100));

        // XXXX Add callbacks.
    }
}

bool TwistTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<TwistedModel>(sel);
}

void TwistTool::Attach() {
    ASSERT(Util::IsA<TwistedModel>(GetModelAttachedTo()));

    // Rotate to match the Model. The TwistTool always aligns with local axes.
    const Vector3f model_size = MatchModelAndGetSize(false);
    const float radius = .6f * ion::math::Length(model_size);

    // Translate the axis rotator handles.
    const Vector3f ytrans(0, radius, 0);
    SG::FindNodeUnderNode(*axis_rotator_, "Min")->SetTranslation(-ytrans);
    SG::FindNodeUnderNode(*axis_rotator_, "Max")->SetTranslation(ytrans);

    // Scale the center translator axis.
    auto scale = center_translator_->GetScale();
    scale[1] = 2 * radius;
    center_translator_->SetScale(scale);

    // Scale and translate the twister parts.
    for (const auto &dim_char: std::string("XZ")) {
        const std::string dim_str(1, dim_char);
        const auto min   = SG::FindNodeUnderNode(*twister_, dim_str + "Min");
        const auto max   = SG::FindNodeUnderNode(*twister_, dim_str + "Max");
        const auto stick = SG::FindNodeUnderNode(*twister_, dim_str + "Stick");
        auto yscale = stick->GetScale();
        yscale[0] = 2 * radius;
        stick->SetScale(yscale);
        const Vector3f xtrans(radius, 0, 0);
        min->SetTranslation(-xtrans);
        max->SetTranslation(xtrans);
    }

    // XXXX
}

void TwistTool::Detach() {
    // Nothing to do here.
}
