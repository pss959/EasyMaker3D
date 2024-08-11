//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <memory>

#include "App/Application.h"
#include "Tests/SceneTestBase.h"

class ScriptedApp;

/// This is a base class for any test that simulates interaction by reading a
/// script from a file and executing the instructions in it, then tests the
/// results. Derived classes must implement the TestResults() function for
/// specific testing.
/// \ingroup Tests
class SimTestBase : public SceneTestBase {
 protected:
    SimTestBase();
    ~SimTestBase();

    /// Derived classes should call this to load and run a script, then call
    /// TestResults() if the script is run successfully. The script name (with
    /// no extension) should be found in the "Scripts" subdirectory of the test
    /// data directory.
    void RunScript(const Str &script_name);

    /// Derived classes must implement this function to check results after
    /// executing the script.
    virtual void TestResults() = 0;

    /// Allows derived classes to access the Application::Context used for the
    /// session. This is useful only after RunScript() is called.
    const Application::Context & GetContext();

  private:
    /// ScriptedApp instance that does most of the work.
    std::unique_ptr<ScriptedApp> app_;

    /// Calls RunScript_() and calls TestResults() if successful. Exits
    /// afterwards.
    void RunScriptAndExit_(const Str &script_name);

    /// Loads and runs the named script, then calls TestResults() if
    /// successful.
    bool RunScript_(const Str &script_name);
};
