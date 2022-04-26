#pragma once

#include <memory>

#include "SceneTestBase.h"
#include "TestContext.h"

class TestingApp_;

// This is a base class for any test that loads a session from a file and tests
// the results.
class SessionTestBase : public SceneTestBase {
 protected:
    // TestContext used for the session.
    TestContext context;

    SessionTestBase();
    ~SessionTestBase();

    // Loads the session from the named file.
    void LoadSession(const std::string &file_name);

  private:
    std::unique_ptr<TestingApp_> app_;
};
