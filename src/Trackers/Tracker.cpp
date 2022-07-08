#include "Trackers/Tracker.h"

#include "Util/Assert.h"
#include "Widgets/Widget.h"

Tracker::Tracker(Actuator actuator) : actuator_(actuator) {
    ASSERT(actuator != Actuator::kNone);
}

void Tracker::SetSceneContext(const SceneContextPtr &context) {
    context_ = context;
}

SceneContext & Tracker::GetContext() const {
    ASSERT(context_);
    return *context_;
}

void Tracker::UpdateHover(const WidgetPtr &old_widget,
                          const WidgetPtr &new_widget) {
    ASSERT(old_widget != new_widget);
    if (old_widget && old_widget->IsHovering())
        old_widget->SetHovering(false);
    if (new_widget)
        new_widget->SetHovering(true);
}
