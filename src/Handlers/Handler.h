#pragma once

#include <memory>

struct Event;

/// Abstract base class for classes that handle Event instances representing
/// input events.
/// \ingroup Interfaces
class Handler {
  public:
    /// Processes the given Event. Returns true if nobody else should get a
    /// chance to handle the event.
    virtual bool HandleEvent(const Event &event) = 0;

    /// Enables or disables the handler. This class implements this to set a
    /// flag. Derived classes may add other functionality. Handlers are enabled
    /// by default.
    virtual void SetEnabled(bool enabled) { is_enabled_ = enabled; }

    /// Returns whether the handler is enabled. This class implements it to
    /// return the flag set in the last call to Enable().
    virtual bool IsEnabled() const { return is_enabled_; }

    /// This returns true to indicate that the derived Handler class wants a
    /// specific state for interactive controller feedback; it sets
    /// show_pointer and show_grip appropriately. If it returns false, the
    /// handler does not care and the out parameters mean nothing. The base
    /// class defines this to return false.
    virtual bool NeedsControllerFeedback(bool &show_pointer,
                                         bool &show_grip) const {
        return false;
    }

    /// Resets the handler, freeing up any resources they may be holding on
    /// to. The base class defines this to do nothing.
    virtual void Reset() {}

  private:
    /// Whether the handler is enabled.
    bool is_enabled_ = true;
};

typedef std::shared_ptr<Handler> HandlerPtr;
