//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

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
      snapimage [--klog=<klog_string>] [--maximize]
                [--nosnap] [--offscreen] [--remain] [--report] SCRIPT

    Options:
      --klog=<string>  String to pass to KLogger::SetKeyString().
      --maximize       Start with a maximized window.
      --nosnap         Ignore snap commands (useful for testing).
      --offscreen      Use offscreen rendering (no visible window).
      --remain         Keep the window alive after script processing.
      --report         Report each instruction of script processing.

    Script files are relative to PublicDoc/scripts/snaps.
    Image files are placed in PublicDoc/docs/images.
)";

int main(int argc, const char *argv[]) {
   Util::app_type = Util::AppType::kInteractive;

    // Let the ScriptedApp handle common options.
    ScriptedApp app;
    const Args args(argc, argv, kUsageString);
    app.InitOptions(args);

    // Set snap-specific ones.
    auto &options  = app.GetOptions();
    options.dryrun = args.GetBool("--nosnap");

    // Process the script.
    return app.ProcessScript("PublicDoc/scripts/snaps",
                             args.GetString("SCRIPT"), false) ? 0 : -1;
}
