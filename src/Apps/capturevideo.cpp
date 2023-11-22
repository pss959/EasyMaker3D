#include <exception>
#include <iostream>
#include <vector>

#include "App/Args.h"
#include "App/CaptureScriptApp.h"
#include "Managers/PanelManager.h"
#include "Panels/FilePanel.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/FilePathList.h"
#include "Util/General.h"
#include "Util/KLog.h"

/// \file
/// The capturevideo application is used to create videos for the public
/// documentation. See the usage string for details.
///
/// \ingroup Apps

constinit const char kUsageString[] =
R"(capturevideo: Play back a session file with delays to create a video for
public documentation.

    Usage:
      capturevideo [--fullscreen] [--klog=<klog_string>]
                   [--nocapture] [--remain] [--report] SCRIPT [SESSION]

    Options:
      --fullscreen    Use a full-screen window.
      --nocapture     Do not actually capture the video (useful for testing).
      --klog=<string> String to pass to KLogger::SetKeyString().
      --remain        Keep the window alive after script processing.
      --report        Report each instruction of script processing.

    The script file is loaded from PublicDoc/videos/scripts/<SCRIPT>.
    If specified, a session is loaded from PublicDoc/videos/sessions/<SESSION>.
    The resulting video file is in "PublicDoc/docs/videos/" and is named the
       same as the SCRIPT with a ".gif" extension.
)";

int main(int argc, const char *argv[]) {
    Util::app_type = Util::AppType::kInteractive;

    Args args(argc, argv, kUsageString);

    CaptureScriptApp::Options options;

    const FilePath path("PublicDoc/videos/scripts/" + args.GetString("SCRIPT"));
    if (! options.script.ReadScript(path))
        return -1;

    std::cout << "======= Processing Script file " << path.ToString() << "\n";

    KLogger::SetKeyString(args.GetString("--klog"));
    options.do_ion_remote      = true;
    options.enable_vr          = false;
    options.fullscreen         = args.GetBool("--fullscreen");
    options.nocapture          = args.GetBool("--nocapture");
    options.remain             = args.GetBool("--remain");
    options.report             = args.GetBool("--report");
    options.show_session_panel = true;

    // Note that this must have the same aspect ratio as fullscreen.
    options.window_size.Set(1024, 552);

    CaptureScriptApp app;
    if (! app.Init(options))
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
