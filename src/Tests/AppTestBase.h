#pragma once

#include <memory>

#include "App/Application.h"
#include "Tests/SceneTestBase.h"

/// This is a base class for any test that requires an Application instance.
/// \ingroup Tests
class AppTestBase : public SceneTestBase {
 protected:
    /// Application::Context used for the session.
    Application::Context context;

    AppTestBase();
    ~AppTestBase();

    // Overrides this to initialize the Application.
    void SetUp() override;

  private:
    class TestingApp_;
    std::unique_ptr<TestingApp_> app_;
};
