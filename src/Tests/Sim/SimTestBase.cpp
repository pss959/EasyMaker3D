#include "Tests/Sim/SimTestBase.h"

#include <exception>
#include <iostream>

#include "App/Script.h"
#include "App/ScriptedApp.h"
#include "Tests/Testing.h"
#include "Tests/UnitTestTypeChanger.h"
#include "Util/FilePath.h"

void SimTestBase::RunScript(const Str &script_name) {
    // Change the app type temporarily.
    UnitTestTypeChanger uttc(Util::AppType::kSimTest);

    // Execute RunScriptAndExit_() as a death test so it exits and cleans up.
    // Ion has some static variables that need to be reset for each execution;
    // calling _exit() is the easiest way to reset everything for each
    // simulation.
    EXPECT_EXIT(RunScriptAndExit_(script_name), testing::ExitedWithCode(0), "");
}

void SimTestBase::RunScriptAndExit_(const Str &script_name) {
    // If the script was executed successfully, let the derived class test the
    // results.
    bool ok = RunScript_(script_name);
    if (ok) {
        TestResults();

        // Detect if anything in TestResults() failed.
        ok = ! ::testing::Test::HasFailure();
    }

    _exit(ok ? 0 : 1);
}

bool SimTestBase::RunScript_(const Str &file_name) {
    ScriptedApp app;
    auto &options = app.GetOptions();

    // Hardwire all options.
    options.do_ion_remote      = false;
    options.ignore_vr          = true;   // Bypasses real VR system init.
    options.enable_vr          = true;   // So controllers work properly.
    options.maximize           = false;
    options.dryrun             = true;
    options.remain             = false;
    options.show_session_panel = false;
    options.offscreen          = true;  // Use hidden window.
    options.window_size.Set(1024, 552);

    app.InitApp();  // Makes the context available.

    // Copy the context.
    context = app.GetContext();

    // Process the script.
    const FilePath script_path = FilePath::Join(
        FilePath::Join(FilePath::GetTestDataPath(), "Scripts"),
        file_name + ".econf");

    return app.ProcessScript(script_path, false);
}
