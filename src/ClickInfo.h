#pragma once

#include "SG/Hit.h"

class ClickableWidget;

//! The ClickInfo struct packages up information about a click from an input
//! device.
struct ClickInfo {
    SG::Hit hit;                //!< The SG::Hit that initiated the click.
    bool    is_long_press;      //!< True if considered a long press.
    bool    is_alternate_mode;  //!< True if in alternate input state.
    ClickableWidget *widget;    //!< Widget the click is on (may be null);
};
