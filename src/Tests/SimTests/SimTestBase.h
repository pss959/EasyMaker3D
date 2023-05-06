#pragma once

#include <string>

#include "App/Application.h"
#include "Tests/SceneTestBase.h"

// This is a base class for any test that simulates interaction by reading a
// script from a file and executing the instructions in it, then tests the
// results.
class SimTestBase : public SceneTestBase {
 protected:
    SimTestBase();

    // Application::Context used for the session. This is useful only after
    // RunScript() is called.
    Application::Context context;

    // Derived classes should call this to load and run a script, then call
    // TestResults() if the script is run successfully. The script name (with
    // no extension) should be found in the "Scripts" subdirectory of the test
    // data directory.
    void RunScript(const std::string &script_name);

    // Derived classes have to implement this function to run the test after
    // executing the script.
    virtual void TestResults() = 0;

  private:
    // Calls RunScript_() and  calls TestResults() if successful. Exits
    // afterwards.
    void RunScriptAndExit_(const std::string &script_name);

    // Loads and runs the named script, then calls TestResults() if successful.
    bool RunScript_(const std::string &script_name);
};
