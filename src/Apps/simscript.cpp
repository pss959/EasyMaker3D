#include <exception>
#include <iostream>

#include "App/Args.h"
#include "App/ScriptedApp.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"

/// \file
/// The simscript application is used to apply a subset of SnapScript
/// instructions to simulate interaction. The resulting scene is accessible to
/// test its state for correctness.
///
/// \ingroup Apps

// ----------------------------------------------------------------------------
// Mainline.
// ----------------------------------------------------------------------------

static const char kUsageString[] =
R"(simscript: Reads a script with instructions to execute to simulate application
 interaction. See SnapScript.h for script details.
    Usage:
      simscript SCRIPT

    Script files are relative to PublicDoc/snaps/scripts.
    Image files are placed in PublicDoc/docs/images.
)";

int main(int argc, const char *argv[]) {
    Args args(argc, argv, kUsageString);

    ScriptedApp::Options options;

    // XXXX Move to ScriptedApp?
    const FilePath path("PublicDoc/snaps/scripts/" + args.GetString("SCRIPT"));
    if (! options.script.ReadScript(path))
        return -1;

    std::cout << "======= Processing Script file " << path.ToString() << "\n";

    // Hardwire all options.
    options.do_ion_remote      = false;
    options.enable_vr          = true;   // So controllers work properly.
    options.fullscreen         = false;
    options.nosnap             = true;
    options.remain             = false;
    options.show_session_panel = false;

    // Note that this must have the same aspect ratio as fullscreen.
    options.window_size.Set(1024, 552);

    ScriptedApp app;
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
