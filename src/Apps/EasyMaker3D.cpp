//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#ifdef ION_PLATFORM_MAC
#include <mach-o/dyld.h>  // For _NSGetExecutablePath()
#endif

#include <csignal>
#include <ctime>
#include <functional>
#include <iostream>
#include <vector>

#include "App/Application.h"
#include "App/Args.h"
#include "Base/Event.h"
#include "Handlers/LogHandler.h"
#include "Math/Types.h"
#include "Models/Model.h"
#include "Util/Assert.h"
#include "Util/ExceptionBase.h"
#include "Util/FilePath.h"
#include "Util/Flags.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/StackTrace.h"
#include "Util/String.h"
#include "Util/Tuning.h"
#include "Util/UTime.h"

/// \file
/// This is the source for the main EasyMaker3D application. See the usage
/// string for details.
///
/// \ingroup Apps

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
    CrashHandler_(IApplication &app) : app_(app) {}
    void HandleSignal(int signal) {
        std::signal(signal, nullptr);
        HandleCrash_("Received signal " + Util::ToString(signal),
                     Util::GetStackTrace());
    }

#if ! RELEASE_BUILD
    void HandleAssertion(const AssertException &ex) {
        HandleCrash_(Str("Caught assertion:\n") + ex.what(),
                     ex.GetStackTrace());
    }
#endif

    void HandleException(const std::exception &ex) {
        StrVec stack;
        if (const ExceptionBase *exb = dynamic_cast<const ExceptionBase *>(&ex))
            stack = exb->GetStackTrace();

        HandleCrash_(Str("Caught exception:\n") + ex.what(), stack);
    }

  private:
    IApplication &app_;

    /// Handles a crash of any type - the cause and stack trace (if available)
    /// are passed in.
    void HandleCrash_(const Str &cause, const StrVec &stack);

    /// Returns a path to a file to save the crash data to.
    static FilePath GetCrashFilePath_();
};

void CrashHandler_::HandleCrash_(const Str &cause, const StrVec &stack) {
#if ! RELEASE_BUILD
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
    return TK::kApplicationName + "_crash_" + buf + TK::kSessionFileExtension;
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

#if RELEASE_BUILD
/// This is used for release builds on the Mac to make sure the working
/// directory is where the app is, allowing the resources to be loaded.
static void SetWorkingDirectory_() {
#ifdef ION_PLATFORM_MAC
    // On the Mac, the executable is run from some unknown place where the DMG
    // was installed. (Or something - not really clear.) Get the executable
    // path to allow resource access.
    uint32_t size = 10000;
    char buf[size];
    _NSGetExecutablePath(buf, &size);
    const FilePath app_path = FilePath(buf).GetAbsolute();

    // The resources directory is in the "Resources" subdirectory of the
    // bundle.
    const FilePath res_path = FilePath::Join(
        app_path.GetParentDirectory().GetParentDirectory(), "Resources");
    res_path.MakeCurrent();
#endif
}
#endif

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
#if ! RELEASE_BUILD
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

constinit const char kUsageString[] =
R"(<NAME>: A VR-enabled application for creating models for 3D printing.
    Usage:
      <NAME> [--klog=<klog_string>] [--maximize] [--novr] [--remote]
             [--session=<filename>] [--touch]

    Options:
      --session=<filename>  EasyMaker3D session file to open by default.

    Debug-only Options:
      --klog=<string>  String to pass to KLogger::SetKeyString().
      --maximize       Start with a maximized window.
      --novr           Simulate non-VR setup even when VR is available.
      --remote         Enable Ion remote debugging (but URLs fail to open).
      --touch          Simulate VR setup for testing touch interaction.
)";

int main(int argc, const char *argv[]) {
    // This is the main application.
    Util::app_type = Util::AppType::kMainApp;

    const Str usage = Util::ReplaceString(kUsageString, "<NAME>",
                                          TK::kApplicationName);
    Args args(argc, argv, usage);

    Application::Options options;

    KLogger::SetKeyString(args.GetString("--klog"));
    options.do_ion_remote     = args.GetBool("--remote");
    options.maximize          = args.GetBool("--maximize");
    options.connect_vr        = ! args.GetBool("--novr");
    options.session_file_name = args.GetString("--session");
    options.set_up_touch      = args.GetBool("--touch");

#if RELEASE_BUILD
    // Release version is always run maximized.
    options.maximize = true;

    SetWorkingDirectory_();
#endif

    Model::EnablePlacementAnimation(true);

    const int height = TK::kWindowHeight;
    const int width  = static_cast<int>(TK::kWindowAspectRatio * height);
    options.window_size.Set(width, height);

    return RunApp_(options) ? 0 : -1;
}
