#pragma once

#include <string>

#include "Event.h"
#include "Math/Types.h"

//! IClickableWidget is an abstract interface class for widgets that react to
//! clicking with some input device.
//! \ingroup Widgets
class IClickableWidget {
  public:
    //! The ClickInfo class packages up information about a click on an
    //! IClickableWidget.
    struct ClickInfo {
        IClickableWidget *widget;       //!< Widget clicked on. May be null.
        Event::Device     device;       //!< Device initiating the click.
        Vector3f    world_point;        //!< Point in world coordinates.
        bool        is_long_press;      //!< True if considered a long press.
        bool        is_alternate_mode;  //!< True if in alternate input state.
    };

    //! Returns the IClickableWidget to use to receive the call to Click() or
    //! anything else that needs to interact with this. This can be used to
    //! substitute another object that needs to get the call to Click(). The
    //! default implementation just returns this.
    virtual IClickableWidget & GetClickable() { return *this; }

    //! Tells the object to respond to a click. A ClickInfo instance containing
    // all relevant click information is passed in.
    virtual void Click(ClickInfo info) = 0;
};
