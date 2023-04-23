#include "Tools/TwistTool.h"

#include <ion/math/vectorutils.h>

#include "Commands/ChangeTwistCommand.h"
#include "Managers/CommandManager.h"
#include "Models/TwistedModel.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Widgets/DiscWidget.h"
#include "Widgets/Slider2DWidget.h"
#include "Widgets/SphereWidget.h"

void TwistTool::CreationDone() {
    Tool::CreationDone();

    if (! IsTemplate()) {
        // Find all of the parts.
        twister_ = SG::FindTypedNodeUnderNode<DiscWidget>(*this, "Twister");
        rotator_ = SG::FindTypedNodeUnderNode<SphereWidget>(*this, "Rotator");
        translator_ = SG::FindTypedNodeUnderNode<Slider2DWidget>(
            *this, "Translator");
        axis_ = SG::FindNodeUnderNode(*translator_, "Axis");
        start_axis_rotation_ = axis_->GetRotation();

        translator_->SetRange(Vector2f(-100, -100), Vector2f(100, 100));

        // Set up callbacks.
        twister_->GetActivation().AddObserver(
            this, [&](Widget &, bool is_act){ Activate_(is_act); });
        twister_->GetRotationChanged().AddObserver(
            this, [&](Widget &, const Anglef &){ TwistChanged_(); });
        rotator_->GetActivation().AddObserver(
            this, [&](Widget &, bool is_act){ Activate_(is_act); });
        rotator_->GetRotationChanged().AddObserver(
            this, [&](Widget &, const Rotationf &){
                axis_->SetRotation(rotator_->GetRotation() *
                                   start_axis_rotation_);
                TwistChanged_();
            });
        translator_->GetActivation().AddObserver(
            this, [&](Widget &, bool is_act){ Activate_(is_act); });
        translator_->GetValueChanged().AddObserver(
            this, [&](Widget &, const Vector2f &){ TwistChanged_(); });
    }
}

bool TwistTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<TwistedModel>(sel);
}

void TwistTool::Attach() {
    static const float kRadiusScale = .75f;
    static const float kAxisScale   = 1.2f;

    // Rotate to match the Model. The TwistTool always aligns with local axes.
    const Vector3f model_size = MatchModelAndGetSize(false);
    const float radius = kRadiusScale * ion::math::Length(model_size);

    // Translate the axis rotator handles.
    const Vector3f ytrans(0, kAxisScale * radius, 0);
    SG::FindNodeUnderNode(*rotator_, "Min")->SetTranslation(-ytrans);
    SG::FindNodeUnderNode(*rotator_, "Max")->SetTranslation(ytrans);

    // Scale the center translator axis (in Z because it is rotated to
    // translate in the XZ plane).
    const auto axis = SG::FindNodeUnderNode(*translator_, "Axis");
    auto scale = axis_->GetScale();
    scale[2] = 2 * kAxisScale * radius;
    axis_->SetScale(scale);

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

    // Match the Twist in the primary TwistedModel without notifying.
    const auto tm = Util::CastToDerived<TwistedModel>(GetModelAttachedTo());
    ASSERT(tm);
    const Twist &twist = tm->GetTwist();
    twister_->GetRotationChanged().EnableObserver(this, false);
    rotator_->GetRotationChanged().EnableObserver(this, false);
    translator_->GetValueChanged().EnableObserver(this, false);
    twister_->SetRotationAngle(twist.angle);
    rotator_->SetRotation(Rotationf::RotateInto(Vector3f::AxisY(), twist.axis));
    translator_->SetValue(Vector2f(twist.center[0], twist.center[1]));
    twister_->GetRotationChanged().EnableObserver(this, true);
    rotator_->GetRotationChanged().EnableObserver(this, true);
    translator_->GetValueChanged().EnableObserver(this, true);

    // Set other geometry.
    axis_->SetRotation(rotator_->GetRotation() * start_axis_rotation_);
}

void TwistTool::Detach() {
    // Nothing to do here.
}

void TwistTool::Activate_(bool is_activation) {
    if (is_activation) {
        const auto tm = Util::CastToDerived<TwistedModel>(GetModelAttachedTo());
        ASSERT(tm);
        start_twist_ = tm->GetTwist();
    }
    else {
        GetDragEnded().Notify(*this);

        // Execute the command to change the ClippedModel(s).
        if (command_) {
            if (command_->GetTwist() != start_twist_)
                GetContext().command_manager->AddAndDo(command_);
            command_.reset();
        }
    }
}

void TwistTool::TwistChanged_() {
    const auto &context = GetContext();

    // If this is the first change, create the ChangeTwistCommand and start the
    // drag.
    if (! command_) {
        command_ = CreateCommand<ChangeTwistCommand>();
        command_->SetFromSelection(GetSelection());
        GetDragStarted().Notify(*this);
    }

    // Create a Twist from the current Widget values and update the command.
    Twist twist;
    twist.center += translator_->GetTranslation();
    twist.axis  = rotator_->GetRotation() * twist.axis;
    twist.angle = twister_->GetRotationAngle();
    command_->SetTwist(twist);
    context.command_manager->SimulateDo(command_);

    // XXXX Add angle feedback.
}
