#include "Tools/TwistTool.h"

#include "Models/TwistedModel.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Widgets/DiscWidget.h"
#include "Widgets/Slider2DWidget.h"
#include "Widgets/SphereWidget.h"

void TwistTool::CreationDone() {
    Tool::CreationDone();

    if (! IsTemplate()) {
        twister_ = SG::FindTypedNodeUnderNode<DiscWidget>(*this, "Twister");
        axis_rotator_ = SG::FindTypedNodeUnderNode<SphereWidget>(
            *this, "AxisRotator");
        center_translator_ = SG::FindTypedNodeUnderNode<Slider2DWidget>(
            *this, "CenterTranslator");

        const float radius = 10; // XXXX TEMP.

        // Translate the axis rotator parts. XXXX Move this to Attach().
        {
            const auto min   = SG::FindNodeUnderNode(*axis_rotator_, "Min");
            const auto max   = SG::FindNodeUnderNode(*axis_rotator_, "Max");
            const Vector3f trans(0, radius, 0);
            min->SetTranslation(-trans);
            max->SetTranslation(trans);
        }

        // Scale the center translator parts. XXXX Move this to Attach().
        {
            auto scale = center_translator_->GetScale();
            scale[1] = 2 * radius;
            center_translator_->SetScale(scale);
            center_translator_->SetRange(Vector2f(-100, -100),
                                         Vector2f(100, 100));
        }

        // Scale and translate the twister parts. XXXX Move this to Attach().
        {
            const auto xmin   = SG::FindNodeUnderNode(*twister_, "XMin");
            const auto xmax   = SG::FindNodeUnderNode(*twister_, "XMax");
            const auto xstick = SG::FindNodeUnderNode(*twister_, "XStick");
            const auto zmin   = SG::FindNodeUnderNode(*twister_, "ZMin");
            const auto zmax   = SG::FindNodeUnderNode(*twister_, "ZMax");
            const auto zstick = SG::FindNodeUnderNode(*twister_, "ZStick");
            auto scale = xstick->GetScale();
            scale[0] = 2 * radius;
            xstick->SetScale(scale);
            zstick->SetScale(scale);
            const Vector3f trans(radius, 0, 0);
            xmin->SetTranslation(-trans);
            zmin->SetTranslation(-trans);
            xmax->SetTranslation(trans);
            zmax->SetTranslation(trans);
        }
    }
}

bool TwistTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<TwistedModel>(sel);
}

void TwistTool::Attach() {
    ASSERT(Util::IsA<TwistedModel>(GetModelAttachedTo()));

    // XXXX DO SOMETHING.
}

void TwistTool::Detach() {
    // Nothing to do here.
}
