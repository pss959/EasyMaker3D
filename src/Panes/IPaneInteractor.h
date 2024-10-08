//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Enums/FocusReason.h"
#include "Util/Memory.h"

struct Event;
DECL_SHARED_PTR(Border);
DECL_SHARED_PTR(ClickableWidget);
DECL_SHARED_PTR(VirtualKeyboard);

/// IPaneInteractor is an interface class for Pane classes that can respond to
/// input events. Any interactive Pane must supply a non-null IPaneInteractor
/// instance from Pane::GetInteractor(). This instance is then used to process
/// events.
///
/// \ingroup Panes
class IPaneInteractor {
  public:
    /// Sets a VirtualKeyboard instance that the Pane can use when needed. The
    /// default does nothing with it.
    virtual void SetVirtualKeyboard(const VirtualKeyboardPtr &vk) {}

    /// Returns a ClickableWidget that can be clicked to focus and activate the
    /// Pane. Returning null means that the only way to activate the Pane is to
    /// navigate focus to it and hit the Enter or Space key. The default is to
    /// return null.
    virtual ClickableWidgetPtr GetActivationWidget() const { return nullptr; }

    /// Returns true if the Pane can be focused for the given reason. The
    /// default is to return true for any reason if GetFocusBorder() returns a
    /// non-null border.
    virtual bool CanFocus(FocusReason reason) const {
        return GetFocusBorder().get();
    }

    /// Returns a Border to highlight to focus the Pane for interaction if
    /// possible, or null if it is not possible. Note that some Pane classes
    /// may respond to interaction only under certain conditions.
    virtual BorderPtr GetFocusBorder() const = 0;

    /// This is called when the Pane is focused or unfocused. The default is to
    /// do nothing.
    virtual void SetFocus(bool is_focused) {}

    /// Activates the Pane. This is called by the Panel when the user hits the
    /// Enter key with the focus on the Pane or if the user clicks on the
    /// activation Widget for the Pane (if it has one). The default is to do
    /// nothing.
    virtual void Activate() {}

    /// Deactivates the Pane. This is called by the Panel when focus is removed
    /// from an interactive Pane that returns true for IsActive(). The default
    /// is to do nothing.
    virtual void Deactivate() {}

    /// Returns true if the Pane should be considered active and does not need
    /// to be activated. The default is always false.
    virtual bool IsActive() const { return false; }

    /// Possibly handles the given Event, returning true if it was handled. The
    /// default is to just return false.
    virtual bool HandleEvent(const Event &event) { return false; }
};
