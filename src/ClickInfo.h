#pragma once

#include "Base/Event.h"
#include "SG/Hit.h"

class ClickableWidget;

/// The ClickInfo struct packages up information about a click from an input
/// device.
struct ClickInfo {
    /// Device that initiated the click.
    Event::Device   device = Event::Device::kUnknown;

    /// The SG::Hit that initiated the click, if any. This may be a
    /// default-constructed SG::Hit if the click is generated by something
    /// other than an event handler.
    SG::Hit         hit;

    /// True if considered a long press.
    bool            is_long_press = false;

    /// True if in alternate input mode.
    bool            is_alternate_mode = false;

    /// Widget the click is on (may be null);
    ClickableWidget *widget;
};
