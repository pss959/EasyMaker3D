#pragma once

#include <memory>

#include "App/Application.h"
#include "Parser/Registry.h"
#include "Tests/SceneTestBase.h"

// This is a base class for any test that requies an Application instance.
class AppTestBase : public SceneTestBase {
 protected:
    // Application::Context used for the session.
    Application::Context context;

    AppTestBase();
    ~AppTestBase();

    /// Creates a Command of the templated and named type.
    template <typename T> static std::shared_ptr<T> CreateCommand() {
        return Parser::Registry::CreateObject<T>();
    }

  private:
    class TestingApp_;
    std::unique_ptr<TestingApp_> app_;
};
