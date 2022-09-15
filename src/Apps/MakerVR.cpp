#include <csignal>
#include <ctime>
#include <functional>
#include <iostream>
#include <vector>

#include "App/Application.h"
#include "App/Args.h"
#include "Base/Event.h"
#include "Base/Tuning.h"
#include "Handlers/LogHandler.h"
#include "Math/Types.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/Flags.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/StackTrace.h"
#include "Util/UTime.h"

namespace {

// This global variable is necessary because std::signal() requires a function
// pointer, not an std::function or lambda.
static std::function<void(int)> s_signal_handler;

// ----------------------------------------------------------------------------
// CrashHandler_ class.
// ----------------------------------------------------------------------------

/// A CrashHandler_ is used to save a session when a signal or exception is
/// detected.
class CrashHandler_ {
  public:
    CrashHandler_(Application &app) : app_(app) {}
    void HandleSignal(int signal) {
        std::signal(signal, nullptr);
        HandleCrash_("Received signal " + Util::ToString(signal),
                     Util::GetStackTrace());
    }

#if ! defined(RELEASE_BUILD)
    void HandleAssertion(const AssertException &ex) {
        HandleCrash_(std::string("Caught assertion: ") + ex.what(),
                     ex.GetStackTrace());
    }
#endif

    void HandleException(const std::exception &ex) {
        HandleCrash_(std::string("Caught exception: ") + ex.what(),
                     StackTrace_());
    }

  private:
    typedef std::vector<std::string> StackTrace_;  ///< Shorthand.

    Application &app_;

    /// Handles a crash of any type - the cause and stack trace (if available)
    /// are passed in.
    void HandleCrash_(const std::string &cause, const StackTrace_ &stack);

    /// Returns a path to a file to save the crash data to.
    static FilePath GetCrashFilePath_();
};

void CrashHandler_::HandleCrash_(const std::string &cause,
                                 const StackTrace_ &stack) {
#if ! defined(RELEASE_BUILD)
    // Non-release version also prints to stderr.
    std::cerr << "*** " << cause << "\n";
    std::cerr << "*** STACK:\n";
    for (const auto &s: stack)
        std::cerr << "  " << s << "\n";
#endif

    app_.SaveCrashSession(GetCrashFilePath_(), cause, stack);
    app_.Shutdown();
}

FilePath CrashHandler_::GetCrashFilePath_() {
    const std::time_t now = std::time(nullptr);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d-%H-%M-%S", std::localtime(&now));
    return std::string(TK::kApplicationName) + "_crash_" +
        buf + TK::kSessionFileSuffix;
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

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

static bool RunApp_(const Application::Options &options) {
    Application app;

    // Saves session when a signal or exception is detected.
    CrashHandler_ ch(app);

    // Detect signals and catch assertions and other exceptions.
    s_signal_handler = [&ch](int signal){ ch.HandleSignal(signal); };
    std::signal(SIGSEGV, [](int signal){ s_signal_handler(signal); });

    try {
        if (! app.Init(options))
            return false;

        // Turn on event logging.
        InitLogging_(app.GetLogHandler());

        app.MainLoop();
    }
#if ! defined(RELEASE_BUILD)
    catch (AssertException &ex) {
        ch.HandleAssertion(ex);
        return false;
    }
#endif
    catch (std::exception &ex) {
        ch.HandleException(ex);
        return false;
    }
    app.Shutdown();
    return true;
}

// ----------------------------------------------------------------------------
// Mainline.
// ----------------------------------------------------------------------------

static const char kUsageString[] =
R"(<NAME>: A VR-enabled application for creating models for 3D printing.
    Usage:
      <NAME> [--fullscreen] [--klog=<klog_string>] [--novr] [--remote] [--touch]

    Debug-only Options:
      --fullscreen    Start with a full-screen window.
      --klog=<string> String to pass to KLogger::SetKeyString().
      --novr          Simulate non-VR setup when VR is available.
      --remote        Enable Ion remote debugging (but URLs fail to open).
      --touch         Simulate VR setup for testing touch interaction.
)";

int main(int argc, const char *argv[]) {
    // This is the main application.
    Util::is_in_main_app = true;

    const std::string usage = Util::ReplaceString(kUsageString, "<NAME>",
                                                  TK::kApplicationName);
    Args args(argc, argv, usage);

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

    return RunApp_(options) ? 0 : -1;
}
