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
    explicit TestingApp_(TestContext &tc);
    void InitForTests();

  private:
    TestContext &tc_;
};

AppTestBase::TestingApp_::TestingApp_(TestContext &tc) : tc_(tc) {
    SetTestingFlag();
}

void AppTestBase::TestingApp_::InitForTests() {
    Application::Options options;
    options.window_size.Set(800, 600);
    Init(options);

    // Have the base Application class fill in the TestContext now that the
    // session is loaded.
    GetTestContext(tc_);

    // Make sure the TestContext has what it needs.
    ASSERT(tc_.scene_context);
    ASSERT(tc_.scene_context->frustum);
}

// ----------------------------------------------------------------------------
// AppTestBase functions.
// ----------------------------------------------------------------------------

AppTestBase::AppTestBase() : app_(new TestingApp_(context)) {
    app_->InitForTests();
}

AppTestBase::~AppTestBase() {
}
