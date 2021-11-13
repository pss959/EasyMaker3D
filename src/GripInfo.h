#pragma once

#include "Event.h"
#include "Math/Types.h"
#include "Widgets/ClickableWidget.h"

/// The GripInfo struct packages up information to help implement grip-hovering
/// and grip-dragging in VR. All but the Event are set by a Grippable object.
struct GripInfo {
    /// Event containing controller position, direction, and motion values.
    Event              event;

    /// Target point of the grip hover feedback in world coordinates.
    Point3f            target_point{0, 0, 0};

    //! Color to use for the grip hover feedback. Defaults to white if not set.
    Color              color = Color::White();

    /// ClickableWidget that is the hover target, or null if there is none.
    ClickableWidgetPtr widget;
};
