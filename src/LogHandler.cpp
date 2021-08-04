#include "LogHandler.h"

#include "Event.h"

LogHandler::LogHandler() {
}

LogHandler::~LogHandler() {
}

bool LogHandler::HandleEvent(const Event &event) {
    if (is_enabled_) {
        std::cout << "=== Event: dev:" << int(event.device) << "\n";
    }
    return false;
}
