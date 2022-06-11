#include <signal.h>

#include <docopt.h>

#include <iostream>
#include <vector>

#include "App/Application.h"
#include "Base/Event.h"
#include "Handlers/LogHandler.h"
#include "Math/Types.h"
#include "Util/Assert.h"
#include "Util/Flags.h"
#include "Util/KLog.h"
#include "Util/StackTrace.h"
#include "Util/VersionInfo.h"

typedef std::map<std::string, docopt::value> DocoptArgs;

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

// Accesses a bool argument from DocoptArgs.
static bool GetBoolArg_(const DocoptArgs &args, const std::string &name) {
    const auto &arg = args.at(name);
    if (arg && arg.isBool())
        return arg.asBool();
    else
        return false;
};

// Accesses a string argument from DocoptArgs.
static std::string GetStringArg_(const DocoptArgs &args,
                                 const std::string &name) {
    const auto &arg = args.at(name);
    if (arg && arg.isString())
        return arg.asString();
    else
        return "";
};

static void InitLogging_(LogHandler &lh) {
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

static bool MainLoop_(const Vector2i &default_window_size, bool do_remote) {
    // Handle segfaults by printing a stack trace.
    signal(SIGSEGV, PrintStack_);

    bool kill_app = false;

    {
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
            throw;   // Rethrow; no use printing a stack for this.
        }
        catch (std::exception &ex) {
            std::cerr << "*** Caught exception:\n" << ex.what() << "\n";
            PrintStack_(0);
        }

        /// \todo Remove this if hang in OpenXR gets fixed.
        kill_app = app.ShouldKillApp();
    }
    if (kill_app) {
        std::cerr << "****** Killing app\n";
        raise(SIGTERM);
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
    DocoptArgs args = docopt::docopt(kUsageString, { argv + 1, argv + argc },
                                     true,         // Show help if requested
                                     "Version " + Util::kVersionString);

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
    //   g:   (GUI) Panel opening and closing.
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
    KLogger::SetKeyString(GetStringArg_(args, "--klog"));

    const bool do_remote = GetBoolArg_(args, "--remote");

    const Vector2i default_size(800, 600);
    return MainLoop_(default_size, do_remote) ? 0 : -1;
}
