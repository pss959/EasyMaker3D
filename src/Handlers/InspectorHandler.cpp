#include "Handlers/InspectorHandler.h"

#include "Base/Event.h"
#include "Base/Tuning.h"
#include "Items/Inspector.h"
#include "Util/Assert.h"

void InspectorHandler::SetInspector(const InspectorPtr &inspector) {
    ASSERT(inspector);
    inspector_ = inspector;
}

bool InspectorHandler::HandleEvent(const Event &event) {
    ASSERT(inspector_);

    // Any key or button press disables the Inspector.
    if (event.flags.Has(Event::Flag::kKeyPress) ||
        event.flags.Has(Event::Flag::kButtonPress)) {
        inspector_->Deactivate();
    }

    // Handle scrolling: scale the inspected object.
    if (event.flags.Has(Event::Flag::kPosition1D)) {
        inspector_->ApplyScaleChange(event.position1D);
    }

    // Handle mouse motion.
    if (event.device == Event::Device::kMouse &&
        event.flags.Has(Event::Flag::kPosition2D)) {
        // Position values are in (0,1) range. Convert to (-1,1) for clarity.
        const Vector2f diff = 2 * event.position2D - Point2f(1, 1);
        const Anglef yaw =
            Anglef::FromDegrees(TK::kInspectorMouseYawAngle   * -diff[0]);
        const Anglef pitch =
            Anglef::FromDegrees(TK::kInspectorMousePitchAngle *  diff[1]);
        inspector_->ApplyRotation(
            Rotationf::FromRollPitchYaw(Anglef(), pitch, yaw));
    }

    // No other handlers can get events while the Inspector is active.
    return true;
}

bool InspectorHandler::IsEnabled() const {
    return Handler::IsEnabled() && inspector_ && inspector_->IsEnabled();
}
