#pragma once

#include "SG/Hit.h"
#include "Widgets/Widget.h"
#include "Util/Notifier.h"

//! ClickableWidget is a base class for widgets that react to clicking with
//! some input device. It reacts to a click by notifying observers of the
//! Notifier returned by GetClicked().
//! \ingroup Widgets
class ClickableWidget : public Widget {
  public:
    //! The ClickInfo class packages up information about a click on an
    //! ClickableWidget.
    struct ClickInfo {
        SG::Hit hit;                //!< The SG::Hit that initiated the click.
        bool    is_long_press;      //!< True if considered a long press.
        bool    is_alternate_mode;  //!< True if in alternate input state.
        ClickableWidget *widget;    //!< Widget handling click (may be null);
    };

    //! Processes a click on this. The base class implements this to do
    //! nothing.
    void Click(const ClickInfo &info) {}
};
