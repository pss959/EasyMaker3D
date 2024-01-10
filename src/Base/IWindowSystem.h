#pragma once

#include <functional>
#include <vector>

#include "Math/Types.h"

struct Event;

/// IWindowSystem is an interface class that abstracts window-system functions.
/// It allows for testing without having to use real windows and event
/// handling.
///
/// \ingroup Base
class IWindowSystem {
  public:
    /// Event options. This is passed to RetrieveEvents().
    struct EventOptions {
        /// Indicates that the emitter should wait for an event if necessary
        /// before emitting events.
        bool wait_for_events = false;

        /// Indicates that mouse motion events should be ignored. It is used by
        /// the snapimage application to avoid spurious events caused by
        /// accidental mouse motion during processing.
        bool ignore_mouse_motion = false;
    };

    using ErrorFunc = std::function<void(const Str &)>;

    /// Initializes the underlying library and sets a function to invoke when
    /// an error is detected by the window system; the function is passed a
    /// string describing the error. Returns false on error.
    virtual bool Init(const ErrorFunc &error_func) = 0;

    /// Finishes use of the window and underlying library.
    virtual void Terminate() = 0;

    /// \name Window management
    ///@{

    /// Creates a window instance. The window will be hidden (useful for
    /// testing) if \p show is false.  Returns false on error. (This should
    /// really be called "CreateWindow", but apparently the fine folks at
    /// Microsoft think it's appropriate to use that name for a macro. Really.)
    virtual bool CreateMainWindow(const Vector2ui &size, const Str &title,
                                  bool show) = 0;

    /// Sets the position of the window.
    virtual void SetWindowPosition(const Point2ui &pos) = 0;

    /// Makes the current window full-screen.
    virtual void SetFullScreen() = 0;

    /// Returns the size of the window in pixels.
    virtual Vector2ui GetWindowSize() = 0;

    /// Returns the size of the window's framebuffer in pixels.
    virtual Vector2ui GetFramebufferSize() = 0;

    ///@}

    /// \name Rendering management
    ///@{

    /// Does whatever is necessary before rendering to the window.
    virtual void PreRender() = 0;

    /// Does whatever is necessary after rendering to the window.
    virtual void PostRender() = 0;

    ///@}

    /// \name Interaction management
    ///@{

    /// Returns true if the user closed the window via a non-event
    /// interaction. This state will be turned into an exit event, so the
    /// derived class can clear the state after checking it.
    virtual bool WasWindowClosed() = 0;

    /// Retrieves all pending events and adds them to the given vector. If \p
    /// wait is true, this should wait for at least one event to become
    /// available. If \p ignore_mouse_motion is true, mouse positions are not
    /// updated in events.
    virtual void RetrieveEvents(const EventOptions &options,
                                std::vector<Event> &events) = 0;

    /// Flushes events that may be pending.
    virtual void FlushPendingEvents() = 0;

    /// Returns true if either shift key is currently pressed.
    virtual bool IsShiftKeyPressed() = 0;

    ///@}
};
