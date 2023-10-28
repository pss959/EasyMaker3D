#pragma once

#include <vector>

#include "Util/Alarm.h"
#include "Util/Memory.h"

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
    /// modified mode should be set in each event is supplied. Returns false if
    /// an event indicates that the application should exit. If the given
    /// max_time (in seconds) is positive and is exceeded during event
    /// processing, unhandled events are pushed onto a queue to be processed
    /// before new ones in the next call. The HasPendingEvents() call will
    /// return true when this happens.
    bool HandleEvents(std::vector<Event> &events, bool is_modified_mode,
                      double max_time);

    /// Returns true if there are events pending due to timeout from the last
    /// call to HandleEvents().
    bool HasPendingEvents() const { return ! pending_events_.empty(); }

    /// Resets all Handlers and resets the serial count for events.
    void Reset();

  private:
    /// Next serial number to use for an emitted event.
    size_t next_serial_ = 0;

    /// Current event Handlers, in order.
    std::vector<HandlerPtr> handlers_;

    /// Pending events that were not handled by HandleEvents() in the allotted
    /// time.
    std::vector<Event> pending_events_;

    /// Does most of the work of HandleEvents(); the passed vector should
    /// contain pending events if there were any.
    bool HandleEvents_(std::vector<Event> &events, bool is_modified_mode,
                       double max_time);

    /// Asks Handlers to handle the given Event. Returns true if the
    /// application should quit.
    bool HandleEvent_(const Event &event);
};
