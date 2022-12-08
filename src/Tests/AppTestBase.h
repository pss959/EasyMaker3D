#pragma once

#include <memory>

#include "Parser/Registry.h"
#include "Tests/SceneTestBase.h"
#include "Tests/TestContext.h"

// This is a base class for any test that requies an Application instance.
class AppTestBase : public SceneTestBase {
 protected:
    // TestContext used for the session.
    TestContext context;

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
