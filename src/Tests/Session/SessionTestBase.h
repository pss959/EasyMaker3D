#pragma once

#include <memory>

#include "App/Application.h"
#include "Tests/SceneTestBase.h"

class Model;

/// Base class for any test that loads an application session from a file and
/// tests the results.
/// \ingroup Tests
class SessionTestBase : public SceneTestBase {
 protected:
    /// Application::Context used for the session.
    Application::Context context;

    SessionTestBase();
    ~SessionTestBase();

    /// Overrides this to initialize the Application.
    virtual void SetUp() override;

    /// Loads a session from the named file (with no extension) in the
    /// "Sessions" subdirectory of the test data directory. Returns false on
    /// error.
    bool LoadSession(const Str &file_name);

    /// Convenience that selects the given Model, which must be found under the
    /// RootModel in the SceneContext.
    void SelectModel(const Model &model);

  private:
    class TestingApp_;
    std::unique_ptr<TestingApp_> app_;
};
