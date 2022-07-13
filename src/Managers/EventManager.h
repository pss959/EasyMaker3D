#pragma once

#include <vector>

#include "Base/Memory.h"

struct Event;
DECL_SHARED_PTR(EventManager);
DECL_SHARED_PTR(Handler);

/// EventManager is responsible for passing events to current handlers.
///
/// \ingroup Managers
class EventManager {
  public:
    /// Adds an event Handler. Handlers are given the opportunity to process
    /// events in the order they are added.
    void AddHandler(const HandlerPtr &handler) {
        handlers_.push_back(handler);
    }

    /// Clears the current list of Handlers.
    void ClearHandlers() { handlers_.clear(); }

    /// Processes the given vector of events. A flag indicating whether
    /// alternate mode should be set in each event is supplied. Returns false
    /// if the event indicates that the application should exit.
    bool HandleEvents(std::vector<Event> &events, bool is_alternate_mode);

    /// Resets all Handlers and resets the serial count for events.
    void Reset();

  private:
    /// Next serial number to use for an emitted event.
    size_t next_serial_ = 0;

    /// Current event Handlers, in order.
    std::vector<HandlerPtr> handlers_;
};
