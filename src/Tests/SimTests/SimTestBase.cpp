#include "SimTests/SimTestBase.h"

#include <exception>
#include <iostream>

#include "App/ScriptedApp.h"
#include "Tests/Testing.h"
#include "Util/FilePath.h"

SimTestBase::SimTestBase() {
    // Override the unit test setting.
    Util::app_type = Util::AppType::kSimTest;
}

void SimTestBase::RunScript(const std::string &script_name) {
    // Execute RunScriptAndExit_() as a death test so it exits and cleans up.
    // Ion has some static variables that need to be reset for each execution;
    // calling _exit() is the easiest way to reset everything for each
    // simulation.
    EXPECT_EXIT(RunScriptAndExit_(script_name), testing::ExitedWithCode(0), "");
}

void SimTestBase::RunScriptAndExit_(const std::string &script_name) {
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

bool SimTestBase::RunScript_(const std::string &file_name) {
    ScriptedApp::Options options;

    // Hardwire all options.
    options.do_ion_remote      = false;
    options.enable_vr          = true;   // So controllers work properly.
    options.fullscreen         = false;
    options.nosnap             = true;
    options.remain             = false;
    options.show_session_panel = false;
    options.window_size.Set(1024, 552);

    // Load the script
    const FilePath path = FilePath::Join(
        FilePath::Join(FilePath::GetTestDataPath(), "Scripts"),
        file_name + ".conf");

    if (! options.script.ReadScript(path))
        return false;

    // Execute the script. Handle exceptions here for better messages.
    ScriptedApp app;
    if (! app.Init(options))
        return false;
    try {
        // Copy the context.
        context = app.GetContext();
        app.MainLoop();
    }
    catch (AssertException &ex) {
        std::cerr << "*** Caught Assertion failure: " << ex.what() << "\n";
        std::cerr << "*** STACK:\n";
        for (const auto &s: ex.GetStackTrace())
            std::cerr << "  " << s << "\n";
        return false;
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught exception: " << ex.what() << "\n";
        return false;
    }

    return true;
}
