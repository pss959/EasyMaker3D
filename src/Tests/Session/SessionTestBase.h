#pragma once

#include <string>

#include "Tests/AppTestBase.h"

class Model;

/// Base class for any test that loads an application session from a file and
/// tests the results.
/// \ingroup Tests
class SessionTestBase : public AppTestBase {
 protected:
    /// Loads a session from the named file (with no extension) in the
    /// "Sessions" subdirectory of the test data directory. Returns false on
    /// error.
    bool LoadSession(const Str &file_name);

    /// Convenience that selects the given Model, which must be found under the
    /// RootModel in the SceneContext.
    void SelectModel(const Model &model);
};
