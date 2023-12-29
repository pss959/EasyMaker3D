#include <exception>
#include <iostream>
#include <vector>

#include "App/Args.h"
#include "App/CaptureScriptApp.h"
#include "App/Script.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/General.h"
#include "Util/KLog.h"

/// \file
/// The capturevideo application is used to create videos for the public
/// documentation. See the usage string for details.
///
/// \ingroup Apps

constinit const char kUsageString[] =
R"(capturevideo: Play back a session file with delays to create a video for
public documentation. See Script.h for script details.

    Usage:
      capturevideo [--format=<str>] [--fps=<fps>] [--klog=<klog_string>]
                   [--nocapture] [--remain] [--report] [--size=<n>]
                   SCRIPT [SESSION]

    Options:
      --format=<str>  Output video format. Choices: "webm" (default),
                        "rgbmp4", "yuvmp4".
      --fps=<long>    Frames per second in the resulting video (default 30).
      --nocapture     Do not actually capture the video (useful for testing).
      --klog=<string> String to pass to KLogger::SetKeyString().
      --remain        Keep the window alive after script processing.
      --report        Report each instruction of script processing.
      --size=<n>      Use fractional (1/n) window size for speed when testing.

    The script file is loaded from PublicDoc/videos/scripts/<SCRIPT>.
    If specified, a session is loaded from PublicDoc/videos/sessions/<SESSION>.
    The resulting video file is in "PublicDoc/docs/extra/videos/" and is named
    the same as the SCRIPT with a different extension.
)";

int main(int argc, const char *argv[]) {
    Util::app_type = Util::AppType::kInteractive;

    Args args(argc, argv, kUsageString);

    const FilePath path("PublicDoc/videos/scripts/" + args.GetString("SCRIPT"));
    std::shared_ptr<Script> script(new Script);
    if (! script->ReadScript(path))
        return -1;

    std::cout << "======= Processing Script file " << path.ToString() << "\n";

    std::shared_ptr<CaptureScriptApp::Options> options(
        new CaptureScriptApp::Options);
    const StrVec formats{ "webm", "rgbmp4", "yuvmp4" };  // Default is first.
    KLogger::SetKeyString(args.GetString("--klog"));
    options->do_ion_remote      = true;
    options->enable_vr          = false;
    options->format             = args.GetStringChoice("--format", formats);
    options->fps                = args.GetAsInt("--fps", 30);
    options->nocapture          = args.GetBool("--nocapture");
    options->remain             = args.GetBool("--remain");
    options->report             = args.GetBool("--report");
    options->show_session_panel = true;

    // Note that this must have the same aspect ratio as fullscreen.
    int size_n = args.GetAsInt("--size", 1);
    if (size_n <= 0)
        size_n = 1;
    options->window_size.Set(1024 / size_n, 552 / size_n);

    CaptureScriptApp app;
    if (! app.Init(options, script))
        return -1;

    try {
        app.MainLoop();
    }
    catch (AssertException &ex) {
        std::cerr << "*** Caught Assertion failure: " << ex.what() << "\n";
        std::cerr << "*** STACK:\n";
        for (const auto &s: ex.GetStackTrace())
            std::cerr << "  " << s << "\n";
        return -1;
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught exception: " << ex.what() << "\n";
        return -1;
    }

    return 0;
}
