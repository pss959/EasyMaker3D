#include "Tools/TwistTool.h"

#include <ion/math/vectorutils.h>

#include "Commands/ChangeTwistCommand.h"
#include "Managers/CommandManager.h"
#include "Math/Twist.h"
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
        axis_rotator_ = SG::FindTypedNodeUnderNode<SphereWidget>(
            *this, "AxisRotator");
        center_translator_ = SG::FindTypedNodeUnderNode<Slider2DWidget>(
            *this, "CenterTranslator");
        rotator_    = SG::FindNodeUnderNode(*this, "Rotator");

        center_translator_->SetRange(Vector2f(-100, -100), Vector2f(100, 100));

        // Set up callbacks.
        twister_->GetActivation().AddObserver(
            this, [&](Widget &, bool is_act){ Activate_(is_act); });
        twister_->GetRotationChanged().AddObserver(
            this, [&](Widget &, const Anglef &){ TwistChanged_(); });
        axis_rotator_->GetActivation().AddObserver(
            this, [&](Widget &, bool is_act){ Activate_(is_act); });
        axis_rotator_->GetRotationChanged().AddObserver(
            this, [&](Widget &, const Rotationf &){
                rotator_->SetRotation(axis_rotator_->GetRotation());
                TwistChanged_();
            });
        center_translator_->GetActivation().AddObserver(
            this, [&](Widget &, bool is_act){ Activate_(is_act); });
        center_translator_->GetValueChanged().AddObserver(
            this, [&](Widget &, const Vector2f &){
                const auto &trans = center_translator_->GetTranslation();
                axis_rotator_->SetTranslation(trans);
                twister_->SetTranslation(trans);
                TwistChanged_();
            });
    }
}

bool TwistTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<TwistedModel>(sel);
}

void TwistTool::Attach() {
    UpdateGeometry_();

    // Reset all Widgets.
    rotator_->SetRotation(Rotationf::Identity());
    axis_rotator_->SetTranslation(Vector3f::Zero());
    twister_->SetTranslation(Vector3f::Zero());

    axis_rotator_->SetRotation(Rotationf::Identity());
    center_translator_->SetValue(Vector2f::Zero());
    twister_->SetRotationAngle(Anglef());
}

void TwistTool::Detach() {
    // Nothing to do here.
}

void TwistTool::ReattachToSelection() {
    // Update without resetting widgets.
    UpdateGeometry_();
}

void TwistTool::UpdateGeometry_() {
    static const float kRadiusScale = .75f;
    static const float kAxisScale   = 1.2f;

    // Rotate to match the Model. The TwistTool always aligns with local axes.
    const Vector3f model_size = MatchModelAndGetSize(false);
    const float radius = kRadiusScale * ion::math::Length(model_size);

    // Translate the axis rotator handles.
    const Vector3f ytrans(0, kAxisScale * radius, 0);
    SG::FindNodeUnderNode(*axis_rotator_, "Min")->SetTranslation(-ytrans);
    SG::FindNodeUnderNode(*axis_rotator_, "Max")->SetTranslation(ytrans);

    // Scale the center translator axis (in Z because it is rotated to
    // translate in the XZ plane).
    auto scale = center_translator_->GetScale();
    scale[2] = 2 * kAxisScale * radius;
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
}

void TwistTool::Activate_(bool is_activation) {
    if (is_activation) {
        // XXXX
    }
    else {
        GetDragEnded().Notify(*this);

        // Execute the command to change the ClippedModel(s).
        if (command_) {
            // XXXX Check for no change.
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
    // XXXX Get center and axis.
    twist.angle = twister_->GetRotationAngle();
    command_->SetTwist(twist);
    context.command_manager->SimulateDo(command_);
}
