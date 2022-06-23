#pragma once

#include <vector>

#include "Base/Memory.h"

struct Event;

DECL_SHARED_PTR(Handler);

/// Abstract base class for classes that handle Event instances representing
/// input events.
///
/// \ingroup Handlers
class Handler {
  public:
    /// Processes the given Event. Returns true if nobody else should get a
    /// chance to handle the event.
    virtual bool HandleEvent(const Event &event) = 0;

    /// This is called immediately after a handler handles an event, giving it
    /// a chance to produce new events to handle before any others. The base
    /// class defines this to return an empty vector.
    virtual std::vector<Event> GetExtraEvents() { return std::vector<Event>(); }

    /// Enables or disables the handler. This class implements this to set a
    /// flag. Derived classes may add other functionality. Handlers are enabled
    /// by default.
    virtual void SetEnabled(bool enabled) { is_enabled_ = enabled; }

    /// Returns whether the handler is enabled. This class implements it to
    /// return the flag set in the last call to Enable().
    virtual bool IsEnabled() const { return is_enabled_; }

    /// Resets the handler, freeing up any resources they may be holding on
    /// to. The base class defines this to do nothing.
    virtual void Reset() {}

  private:
    /// Whether the handler is enabled.
    bool is_enabled_ = true;
};
