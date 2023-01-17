#pragma once

#include "Tests/AppTestBase.h"

// This is a base class for any test that loads a session from a file and tests
// the results.
class SessionTestBase : public AppTestBase {
 protected:
    SessionTestBase();

    // Loads the session from the named file (with no extension). Returns false
    // on error.
    bool LoadSession(const std::string &file_name);
};
