#include "Managers/EventManager.h"

#include "Base/Event.h"
#include "Handlers/Handler.h"

bool EventManager::HandleEvents(std::vector<Event> &events,
                                bool is_alternate_mode) {
    bool keep_going = true;
    for (auto &event: events) {
        event.serial             = next_serial_++;
        event.is_alternate_mode = is_alternate_mode;

        for (auto &handler: handlers_) {
            if (handler->HandleEvent(event))
                break;

            if (event.flags.Has(Event::Flag::kExit)) {
                keep_going = false;
                break;
            }
        }
    }
    return keep_going;
}
