#include <signal.h> // TODO: Remove when not needed.

#include <iostream>
#include <vector>

#include <ion/math/vector.h>

#include "Application.h"
#include "Event.h"
#include "Handlers/LogHandler.h"
#include "Util/Flags.h"

using ion::math::Vector2i;

static void InitLogging(LogHandler &lh) {
    // Uncomment this to start with event logging enabled.
    // lh.Enable(true);

    // Uncomment this to filter by devices.
    // lh.SetDevices({ Event::Device::kKeyboard });

    // Uncomment this to filter by flags.
    // Flags<Event::Flag> flags;
    // flags.Set(Event::Flag::kKeyPress);
    // lh.SetFlags(flags);
}

static bool MainLoop(const Vector2i &default_window_size) {
    Application app;
    app.Init(default_window_size);
    IApplication::Context &context = app.GetContext();
    if (context.viewers.empty())
        return false;

    // Turn on event logging.
    InitLogging(app.GetLogHandler());

    app.MainLoop();

    // TODO: Remove this if hang in OpenXR gets fixed.
    if (app.ShouldKillApp()) {
        std::cerr << "****** Killing app\n";
        raise(SIGTERM);
    }

    return true;
}

int main() {
    const Vector2i default_size(800, 600);
    return MainLoop(default_size) ? 0 : -1;
}
