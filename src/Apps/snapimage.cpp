#include <iostream>

#include "App/Args.h"
#include "App/Script.h"
#include "App/ScriptedApp.h"
#include "Util/FilePath.h"
#include "Util/General.h"
#include "Util/KLog.h"

/// \file
/// The snapimage application is used to create snapshot images for the public
/// documentation. See the usage string for details.
///
/// \ingroup Apps

// ----------------------------------------------------------------------------
// Mainline.
// ----------------------------------------------------------------------------

constinit const char kUsageString[] =
R"(snapimage: Reads a script with instructions on how to create snapshot images
for public documentation. See Script.h for script details.
    Usage:
      snapimage [--fullscreen] [--klog=<klog_string>]
                [--nosnap] [--remain] [--report] SCRIPT

    Options:
      --fullscreen    Start with a full-screen window.
      --klog=<string> String to pass to KLogger::SetKeyString().
      --nosnap        Ignore snap commands (useful for testing).
      --remain        Keep the window alive after script processing.
      --report        Report each instruction of script processing.

    Script files are relative to PublicDoc/snaps/scripts.
    Image files are placed in PublicDoc/docs/images.
)";

int main(int argc, const char *argv[]) {
   Util::app_type = Util::AppType::kInteractive;

    // Let the ScriptedApp handle common options.
    ScriptedApp app;
    const Args args(argc, argv, kUsageString);
    app.InitOptions(args);

    // Set snap-specific ones.
    auto &options = app.GetOptions();
    options.dryrun             = args.GetBool("--nosnap");
    options.show_session_panel = false;

    // Process the script.
    const FilePath script_path("PublicDoc/snaps/scripts/" +
                               args.GetString("SCRIPT"));
    return app.ProcessScript(script_path) ? 0 : -1;
}
