#include <signal.h>

#include <iostream>
#include <vector>

#include "Application.h"
#include "Assert.h"
#include "Event.h"
#include "Handlers/LogHandler.h"
#include "Math/Types.h"
#include "Util/Flags.h"
#include "Util/KLog.h"
#include "Util/StackTrace.h"

static void InitLogging(LogHandler &lh) {
    lh.SetEnabled(KLogger::HasKeyCharacter('e'));

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

    bool kill_app = false;

    {
        Application app;

        // Do the same for exceptions.
        try {
            if (! app.Init(default_window_size))
                return false;

            // Turn on event logging.
            InitLogging(app.GetLogHandler());

            app.MainLoop();

        }
        catch (AssertException &ex) {
            std::cerr << "*** Caught assertion exception:\n"
                      << ex.what() << "\n";
            throw;   // Rethrow; no use printing a stack for this.
        }
        catch (std::exception &ex) {
            std::cerr << "*** Caught exception:\n" << ex.what() << "\n";
            PrintStack_(0);
        }

        // TODO: Remove this if hang in OpenXR gets fixed.
        kill_app = app.ShouldKillApp();
    }
    if (kill_app) {
        std::cerr << "****** Killing app\n";
        raise(SIGTERM);
    }

    return true;
}

int main() {
    // Set up the debug logging key string.
    // Character codes:
    //   b:   Scene graph bounds computation.
    //   c:   Scene graph object construction and destruction.
    //   e:   Events.
    //   h:   MainHandler state changes.
    //   i:   Intersection testing in the scene.
    //   k:   Clicks on objects.
    //   m:   Changes to matrices in SG Nodes.
    //   n:   Notification.
    //   o:   Parser object name scoping and resolution.
    //   p:   Pane sizing.
    //   r:   Ion registries.
    //   u:   Ion uniform processing.
    KLogger::SetKeyString("");

    const Vector2i default_size(800, 600);
    return MainLoop(default_size) ? 0 : -1;
}
