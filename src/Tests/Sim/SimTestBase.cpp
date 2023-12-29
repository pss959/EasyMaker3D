#include "Tests/Sim/SimTestBase.h"

#include <exception>
#include <iostream>

#include "App/SnapScript.h"
#include "App/SnapScriptApp.h"
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
    // Change the app type temporarily.
    UnitTestTypeChanger uttc(Util::AppType::kSimTest);

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
    std::shared_ptr<SnapScriptApp::Options> options(new SnapScriptApp::Options);

    // Hardwire all options.
    options->do_ion_remote      = false;
    options->ignore_vr          = true;  // Bypasses real VR system init.
    options->enable_vr          = true;  // So controllers work properly.
    options->fullscreen         = false;
    options->nosnap             = true;
    options->remain             = false;
    options->show_session_panel = false;
    options->window_size.Set(1024, 552);

    // Load the script
    const FilePath path = FilePath::Join(
        FilePath::Join(FilePath::GetTestDataPath(), "Scripts"),
        file_name + ".conf");
    std::shared_ptr<SnapScript> script(new SnapScript);
    if (! script->ReadScript(path))
        return false;

    // Execute the script. Handle exceptions here for better messages.
    SnapScriptApp app;
    if (! app.Init(options, script))
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
