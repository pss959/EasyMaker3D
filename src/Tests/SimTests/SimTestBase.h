#pragma once

#include <functional>
#include <string>

#include "App/Application.h"
#include "Tests/SceneTestBase.h"

// This is a base class for any test that simulates interaction by reading a
// script from a file and executing the instructions in it, then tests the
// results.
class SimTestBase : public SceneTestBase {
 protected:
    // Application::Context used for the session. This is useful only after
    // RunScript() is called.
    Application::Context context;

    SimTestBase();
    virtual ~SimTestBase();

    // Loads a script from the named file (with no extension) in the "Scripts"
    // subdirectory of the test data directory and runs it. Returns false on
    // error. The results can be tested after this call; the destructor ends
    // the simulation.
    bool RunScript(const std::string &file_name);
};
