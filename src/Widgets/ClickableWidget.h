#pragma once

#include "Widgets/Widget.h"

struct ClickInfo;

//! ClickableWidget is a base class for widgets that react to clicking with
//! some input device. It reacts to a click by notifying observers of the
//! Notifier returned by GetClicked().
//! \ingroup Widgets
class ClickableWidget : public Widget {
  public:
    //! Processes a click on this. The base class implements this to do
    //! nothing.
    void Click(const ClickInfo &info) {}
};
