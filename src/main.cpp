#include <signal.h>

#include <iostream>
#include <vector>

#include "Application.h"
#include "Event.h"
#include "Handlers/LogHandler.h"
#include "Math/Types.h"
#include "Util/Flags.h"
#include "Util/KLog.h"
#include "Util/StackTrace.h"

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

static void PrintStack_(int sig) {
  if (sig != 0)
      fprintf(stderr, "*** Error: signal %d:\n", sig);
  Util::PrintStackTrace();
  exit(1);
}

static bool MainLoop(const Vector2i &default_window_size) {
    // Handle segfaults by printing a stack trace.
    signal(SIGSEGV, PrintStack_);

    Application app;

    // Do the same for exceptions.
    try {
        app.Init(default_window_size);
        IApplication::Context &context = app.GetContext();
        if (context.viewers.empty())
            return false;

        // Turn on event logging.
        InitLogging(app.GetLogHandler());

        app.MainLoop();

    } catch (std::exception &ex) {
        std::cerr << "*** Caught exception: " << ex.what() << "\n";
        PrintStack_(0);
    }

    // TODO: Remove this if hang in OpenXR gets fixed.
    if (app.ShouldKillApp()) {
        std::cerr << "****** Killing app\n";
        raise(SIGTERM);
    }

    return true;
}

int main() {
    // Set up the debug logging key string.
    // Character codes:
    //   c:   Scene graph object construction and destruction.
    //   k:   Clicks on objects.
    //   n:   Notification.
    KLogger::SetKeyString("");

    const Vector2i default_size(800, 600);
    return MainLoop(default_size) ? 0 : -1;
}
