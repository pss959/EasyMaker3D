#include <signal.h> // XXXX

#include <iostream>
#include <vector>

#include <ion/math/vector.h>

#include "Application.h"
#include "Event.h"
#include "Interfaces/IEmitter.h"
#include "Interfaces/IHandler.h"
#include "Interfaces/IViewer.h"

using ion::math::Vector2i;

static bool MainLoop(const Vector2i &default_window_size) {
    Application app;
    IApplication::Context &context = app.Init(default_window_size);
    if (context.viewers.empty())
        return false;

    std::vector<Event> events;
    bool keep_running = true;
    while (keep_running) {
        // Handle all incoming events.
        events.clear();
        for (auto &emitter: context.emitters)
            emitter->EmitEvents(events);
        for (auto &event: events) {
            // Special case for exit events.
            if (event.HasFlag(Event::Flag::kExit)) {
                keep_running = false;
                break;
            }
            for (auto &handler: context.handlers)
                if (handler->HandleEvent(event))
                    break;
        }

        // Render to all viewers.
        for (auto &viewer: context.viewers)
            viewer->Render(*context.scene, *context.renderer);
    }

    if (app.ShouldKillApp()) {
        std::cerr << "XXXX Killing app\n";
        raise(SIGTERM); // XXXX Force exit, since SteamVR hangs!
    }

    return true;
}

int main() {
    const Vector2i default_size(800, 600);
    return MainLoop(default_size) ? 0 : -1;
}
