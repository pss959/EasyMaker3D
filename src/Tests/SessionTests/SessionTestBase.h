#pragma once

#include "SceneTestBase.h"

// This is a base class for any test that loads a session from a file and tests
// the results.
class SessionTestBase : public SceneTestBase {
 protected:
    SessionTestBase();
    ~SessionTestBase();

    // Loads the session from the named file.
    void LoadSession(const std::string &file_name);
};
