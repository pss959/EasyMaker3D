//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Util/Memory.h"
#include "Util/Notifier.h"
#include "Widgets/Widget.h"

struct ClickInfo;

DECL_SHARED_PTR(ClickableWidget);

/// ClickableWidget is a base class for widgets that react to clicking with
/// some input device. It reacts to a click by notifying observers of the
/// Notifier returned by GetClicked().
///
/// \ingroup Widgets
class ClickableWidget : public Widget {
  public:
    /// Returns a Notifier that is invoked when the Click() function is
    /// called. The event is passed a ClickInfo instance containing all
    /// relevant data.
    Util::Notifier<const ClickInfo &> & GetClicked() {
        return clicked_;
    }

    /// Processes a click on this. The base class implements this to just
    /// notify through the GetClicked() notifier.
    virtual void Click(const ClickInfo &info) {
        clicked_.Notify(info);
    }

  protected:
    /// The constructor is protected to make this abstract.
    ClickableWidget() {}

  private:
    /// Notifies when a click is detected.
    Util::Notifier<const ClickInfo &> clicked_;
};
