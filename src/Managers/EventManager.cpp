#include "Managers/EventManager.h"

#include "Base/Event.h"
#include "Handlers/Handler.h"
#include "Util/KLog.h"
#include "Util/String.h"

bool EventManager::HandleEvents(std::vector<Event> &events,
                                bool is_alternate_mode) {
    bool keep_going = true;
    for (auto &event: events) {
        event.serial             = next_serial_++;
        event.is_alternate_mode = is_alternate_mode;

        for (auto &handler: handlers_) {
            if (handler->IsEnabled() && handler->HandleEvent(event)) {
                KLOG('e', "Event handled by "
                     << Util::Demangle(typeid(*handler).name()));
                break;
            }

            if (event.flags.Has(Event::Flag::kExit)) {
                keep_going = false;
                break;
            }
        }
    }
    return keep_going;
}

void EventManager::Reset() {
    for (auto &handler: handlers_)
        handler->Reset();
    next_serial_ = 0;
}
