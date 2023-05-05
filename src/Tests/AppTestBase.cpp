#include "Tests/AppTestBase.h"

#include "App/Application.h"
#include "Managers/SceneContext.h"
#include "Util/Assert.h"

// ----------------------------------------------------------------------------
// Derived Application class that provides functions for testing via the
// Application's protected interface.
// ----------------------------------------------------------------------------

class AppTestBase::TestingApp_ : public Application {
  public:
    TestingApp_();
    void InitForTests();

    // Make this available to AppTestBase.
    using Application::GetContext;
};

AppTestBase::TestingApp_::TestingApp_() {
    SetTestingFlag();
}

void AppTestBase::TestingApp_::InitForTests() {
    Application::Options options;
    options.window_size.Set(800, 600);
    Init(options);

    // Make sure the TestContext has what it needs.
    ASSERT(GetContext().scene_context);
    ASSERT(GetContext().scene_context->frustum);
}

// ----------------------------------------------------------------------------
// AppTestBase functions.
// ----------------------------------------------------------------------------

AppTestBase::AppTestBase() : app_(new TestingApp_) {
    app_->InitForTests();

    // Copy Application::Context locally for convenience in derived classes.
    context = app_->GetContext();
}

AppTestBase::~AppTestBase() {
}
