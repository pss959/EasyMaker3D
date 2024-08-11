//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include <exception>
#include <iostream>
#include <vector>

#include "App/Args.h"
#include "App/ScriptedApp.h"
#include "Util/FilePath.h"
#include "Util/General.h"

/// \file
/// The capturevideo application is used to create videos for the public
/// documentation. See the usage string for details.
///
/// \ingroup Apps

constinit const char kUsageString[] =
R"(capturevideo: Use a script with timing information to create a video for
public documentation. See Script.h for script details.

    Usage:
      capturevideo [--format=<str>] [--fps=<fps>] [--klog=<klog_string>]
                   [--maximize] [--nocapture] [--offscreen]
                   [--remain] [--report] [--size=<n>] SCRIPT

    Options:
      --format=<str>   Output video format. Choices: "webm" (default),
                        "rgbmp4", "yuvmp4".
      --fps=<long>     Frames per second in the resulting video (default 30).
      --klog=<string>  String to pass to KLogger::SetKeyString().
      --maximize       Start with a maximized window.
      --nocapture      Do not actually capture the video (useful for testing).
      --offscreen      Use offscreen rendering (no visible window).
      --remain         Keep the window alive after script processing.
      --report         Report each instruction of script processing.
      --size=<n>       Use fractional (1/n) window size for speed when testing.

    The script file is loaded from PublicDoc/scripts/videos/<SCRIPT>. The
    resulting video file is put in "PublicDoc/docs/extra/videos/" and is named
    the same as the SCRIPT with a different extension.  )";

int main(int argc, const char *argv[]) {
    Util::app_type = Util::AppType::kInteractive;

    // Let the ScriptedApp handle common options.
    ScriptedApp app;
    const Args args(argc, argv, kUsageString);
    app.InitOptions(args);

    // Set video-specific ones.
    auto &options = app.GetOptions();
    const StrVec formats{ "webm", "rgbmp4", "yuvmp4" };  // Default is first.
    options.dryrun    = args.GetBool("--nocapture");
    options.fps       = args.GetAsInt("--fps", 30);
    options.vidformat = args.GetStringChoice("--format", formats);

    // Process the script.
    return app.ProcessScript("PublicDoc/scripts/videos",
                             args.GetString("SCRIPT"), true) ? 0 : -1;
}
