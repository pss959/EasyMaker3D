#pragma once

#include "Base/Memory.h"

struct Event;
DECL_SHARED_PTR(ClickableWidget);

/// IPaneInteractor is an interface class for Pane classes that can respond to
/// input events. Any interactive Pane must supply a non-null IPaneInteractor
/// instance from Pane::GetInteractor(). This instance is then used to process
/// events.
///
/// \ingroup Panes
class IPaneInteractor {
  public:
    /// Returns a ClickableWidget that can be clicked to focus and activate the
    /// Pane. Returning null means that the only way to activate the Pane is to
    /// navigate focus to it and hit the Enter or Space key. The default is to
    /// return null.
    virtual ClickableWidgetPtr GetActivationWidget() const { return nullptr; }

    /// Returns true if the Pane can be focused for interaction. Some Panes
    /// respond to interaction only under certain conditions.
    virtual bool CanFocus() const = 0;

    /// This is called when the Pane is focused or unfocused. The default is to
    /// do nothing.
    virtual void SetFocus(bool is_focused) {}

    /// Activates the Pane. This is called by the Panel when the user hits the
    /// Enter key with the focus on the Pane or if the user clicks on the
    /// activation Widget for the Pane (if it has one). The default is to do
    /// nothing.
    virtual void Activate() {}

    /// Returns true if the Pane should be considered active and does not need
    /// to be activated. The default is always false.
    virtual bool IsActive() const { return false; }

    /// Possibly handles the given Event, returning true if it was handled. The
    /// default is to just return false.
    virtual bool HandleEvent(const Event &event) { return false; }
};
