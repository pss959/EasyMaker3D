#include <iostream>
#include <vector>

#include "App/Application.h"
#include "App/Args.h"
#include "Base/Event.h"
#include "Handlers/LogHandler.h"
#include "Math/Types.h"
#include "Util/Assert.h"
#include "Util/Flags.h"
#include "Util/KLog.h"
#include "Util/StackTrace.h"
#include "Util/VersionInfo.h"

static void InitLogging_(LogHandler &lh) {
    lh.SetEnabled(KLogger::HasKeyCharacter('e'));

    // Uncomment this to filter by devices.
    // lh.SetDevices({ Event::Device::kKeyboard });

    // Uncomment this to filter by flags.
    /**/
    Util::Flags<Event::Flag> flags;
    flags.Set(Event::Flag::kKeyPress);
    flags.Set(Event::Flag::kKeyRelease);
    flags.Set(Event::Flag::kButtonPress);
    flags.Set(Event::Flag::kButtonRelease);
    lh.SetFlags(flags);
    /**/
}

static void PrintStack_(int sig) {
    if (sig != 0)
        fprintf(stderr, "*** Error: signal %d:\n", sig);
    Util::PrintStackTrace();
    exit(1);
}

static bool MainLoop_(const Vector2i &default_window_size, bool do_remote) {
    Application app;

    // Do the same for exceptions.
    try {
        if (! app.Init(default_window_size, do_remote))
            return false;

        // Turn on event logging.
        InitLogging_(app.GetLogHandler());

        app.MainLoop();
    }
    catch (AssertException &ex) {
        std::cerr << "*** Caught assertion exception:\n"
                  << ex.what() << "\n";
        app.Shutdown();
        throw;   // Rethrow; no use printing a stack for this.
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught exception:\n" << ex.what() << "\n";
        PrintStack_(0);
        app.Shutdown();
        return false;
    }

    return true;
}

static const char kUsageString[] =
R"(imakervr: A VR-enabled application for creating models for 3D printing.

    Usage:
      imakervr [--klog=<klog_string>] [--remote]

    Options:
      --klog=<string> Extra string to pass to KLogger::SetKeyString().
      --remote        Enable Ion remote debugging (but URLs fail to open).
)";

int main(int argc, const char *argv[]) {
    Args args(argc, argv, kUsageString);

    // Set up the debug logging key string.
    // Character codes:
    //   a:   NameManager name processing.
    //   A:   SG::Tracker data storage and lookup.
    //   b:   Scene graph bounds computation.
    //   B:   Model mesh building and invalidation.
    //   c:   Scene graph object construction and destruction.
    //   d:   Feedback activation and deactivation.
    //   e:   Events and event handling.
    // + f:   File parsing [Should be set before parsing scene file!].
    //   F:   Interactive Pane focus and activation.
    //   g:   (GUI) Board and Panel opening and closing.
    //   h:   MainHandler state changes.
    //   i:   Intersection testing in the scene.
    //   I:   Ion setup for SG nodes.
    //   k:   Clicks on objects.
    //   m:   Changes to matrices in SG Nodes.
    //   M:   Model structure and visibility changes.
    //   n:   Initial notification trigger.
    //   N:   All notifications.
    //   o:   Notification observer changes.
    //   p:   Pane sizing.
    // + P:   Object parsing.
    //   q:   Pane size notification.
    //   r:   Ion registries.
    //   R:   Rendering.
    // + s:   Parser name scoping and resolution.
    //   S:   Selection changes.
    //   t:   Threads for delayed execution.
    //   T:   Tool initialization, selection, and attachment.
    //   u:   Ion uniform processing.
    //   v:   VR system status.
    //   w:   Session reading and writing.
    //   x:   Command execution, undo, redo.
    //   y:   Model status changes.
    //   z:   Font loading.
    //
    // Special characters:
    //   !:   Disables all logging. <Alt>! in the app toggles this.
    //   *:   Enables all characters. Use with caution!.
    //
    // Codes tagged with a '+' are better set up before parsing the scene file
    // by using the '--klog' option.
    KLogger::SetKeyString(args.GetString("--klog"));

    const bool do_remote = args.GetBool("--remote");

    const float kHDAspect = 16.f / 9.f;
    const Vector2i default_size(static_cast<int>(600 * kHDAspect), 600);
    return MainLoop_(default_size, do_remote) ? 0 : -1;
}
