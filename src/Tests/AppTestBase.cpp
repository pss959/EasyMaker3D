#include "Tests/AppTestBase.h"

#include "App/Application.h"
#include "Managers/SceneContext.h"
#include "Util/Assert.h"
#include "Util/General.h"

/// Derived Application class that provides functions for testing via the
/// Application's protected interface.
/// \ingroup Tests
class AppTestBase::TestingApp_ : public Application {
  public:
    void InitForTests();

    // Make this available to AppTestBase.
    using Application::GetContext;
};

void AppTestBase::TestingApp_::InitForTests() {
    Application::Options options;
    options.window_size.Set(800, 600);
    Init(options);

    // Make sure the TestContext has what it needs.
    ASSERT(GetContext().scene_context);
    ASSERT(GetContext().scene_context->frustum);
}

AppTestBase::AppTestBase() : app_(new TestingApp_) {
    Util::app_type = Util::AppType::kUnitTest;
}

AppTestBase::~AppTestBase() {
}

void AppTestBase::SetUp() {
    app_->InitForTests();

    // Copy Application::Context locally for convenience in derived classes.
    context = app_->GetContext();
}
