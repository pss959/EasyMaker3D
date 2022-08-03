#include <iostream>
#include <vector>

#include "App/Application.h"
#include "App/Args.h"
#include "Base/Event.h"
#include "Base/Tuning.h"
#include "Handlers/LogHandler.h"
#include "Math/Types.h"
#include "Util/Assert.h"
#include "Util/Flags.h"
#include "Util/KLog.h"
#include "Util/StackTrace.h"

static void InitLogging_(LogHandler &lh) {
    lh.SetEnabled(KLogger::HasKeyCharacter('e'));

    // Uncomment this to filter by devices.
    // lh.SetDevices({ Event::Device::kKeyboard });

    // Uncomment this to filter by flags.
    /*
    Util::Flags<Event::Flag> flags;
    flags.Set(Event::Flag::kKeyPress);
    flags.Set(Event::Flag::kKeyRelease);
    flags.Set(Event::Flag::kButtonPress);
    flags.Set(Event::Flag::kButtonRelease);
    lh.SetFlags(flags);
    */
}

static void PrintStack_(int sig) {
    if (sig != 0)
        fprintf(stderr, "*** Error: signal %d:\n", sig);
    Util::PrintStackTrace();
    exit(1);
}

static bool MainLoop_(const Application::Options &options) {
    Application app;

    // Do the same for exceptions.
    try {
        if (! app.Init(options))
            return false;

        // Turn on event logging.
        InitLogging_(app.GetLogHandler());

        app.MainLoop();
    }
    catch (AssertException &ex) {
        std::cerr << "*** Caught assertion exception:\n" << ex.what() << "\n";
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
      imakervr [--fullscreen] [--klog=<klog_string>] [--novr]
               [--remote] [--touch]

    Debug-only Options:
      --fullscreen    Start with a full-screen window.
      --klog=<string> Extra string to pass to KLogger::SetKeyString().
      --novr          Simulate non-VR setup when VR is available.
      --remote        Enable Ion remote debugging (but URLs fail to open).
      --touch         Simulate VR setup for testing touch interaction.
)";

int main(int argc, const char *argv[]) {
    Args args(argc, argv, kUsageString);

    Application::Options options;

    KLogger::SetKeyString(args.GetString("--klog"));
    options.do_ion_remote = args.GetBool("--remote");
    options.fullscreen    = args.GetBool("--fullscreen");
    options.ignore_vr     = args.GetBool("--novr");
    options.set_up_touch  = args.GetBool("--touch");

#if RELEASE_BUILD
    // Release version is always run fullscreen.
    options.fullscreen = true;
#endif

    const int height = TK::kWindowHeight;
    const int width  = static_cast<int>(TK::kWindowAspectRatio * height);
    options.window_size.Set(width, height);

    return MainLoop_(options) ? 0 : -1;
}
