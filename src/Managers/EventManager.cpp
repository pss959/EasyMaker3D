#include "Managers/EventManager.h"

#include "Base/Event.h"
#include "Handlers/Handler.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/String.h"

bool EventManager::HandleEvents(std::vector<Event> &events,
                                bool is_modified_mode, double max_time) {
    // If there are events pending from before, create a concatenated vector.
    if (HasPendingEvents()) {
        std::vector<Event> all_events = pending_events_;
        Util::AppendVector(events, all_events);
        pending_events_.clear();
        return HandleEvents_(all_events, is_modified_mode, max_time);
    }
    else {
        return HandleEvents_(events, is_modified_mode, max_time);
    }
}

void EventManager::Reset() {
    for (auto &handler: handlers_)
        handler->Reset();
    next_serial_ = 0;
    pending_events_.clear();
}

bool EventManager::HandleEvents_(std::vector<Event> &events,
                                 bool is_modified_mode, double max_time) {
    Alarm alarm;
    bool keep_going = true;
    if (max_time > 0)
        alarm.Start(max_time);

    const size_t event_count = events.size();
    for (size_t i = 0; i < event_count; ++i) {
        Event &event = events[i];
        event.serial            = next_serial_++;

        // If is_modified_mode is already set, leave it alone.
        if (! event.is_modified_mode)
            event.is_modified_mode = is_modified_mode;

        if (HandleEvent_(event)) {
            keep_going = false;
            break;
        }

        // If time runs out, just add the remaining events to the pending
        // vector.
        if (max_time > 0 && alarm.IsFinished() && i + 1 < event_count) {
            KLOG('E', "Deferring " << (event_count - (i + 1)) << " event(s)");
            pending_events_.insert(pending_events_.begin(),
                                   events.begin() + (i + 1), events.end());
            break;
        }
    }
    return keep_going;
}

bool EventManager::HandleEvent_(const Event &event) {
    KLOG('e', event.ToString());
    for (auto &handler: handlers_) {
        if (handler->IsEnabled() && handler->HandleEvent(event)) {
            KLOG('e', "Event handled by "
                 << Util::Demangle(typeid(*handler).name()));
            break;
        }

        // Check for application exit.
        if (event.flags.Has(Event::Flag::kExit))
            return true;
    }
    return false;
}
