#pragma once

#include <string>

#include "Tests/AppTestBase.h"

class Model;

// This is a base class for any test that loads a session from a file and tests
// the results.
class SessionTestBase : public AppTestBase {
 protected:
    // Loads a session from the named file (with no extension) in the
    // "Sessions" subdirectory of the test data directory. Returns false on
    // error.
    bool LoadSession(const std::string &file_name);

    // Convenience that selects the given Model, which must be found under the
    // RootModel in the SceneContext.
    void SelectModel(const Model &model);
};
