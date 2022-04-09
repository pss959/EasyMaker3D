#include "SessionTests/SessionTestBase.h"

#include "Application.h"
#include "Managers/SessionManager.h"
#include "Math/Types.h"
#include "Testing.h"
#include "Util/FilePath.h"

// ----------------------------------------------------------------------------
// Derived Application class that provides functions for testing via the
// Application's protected interface.
// ----------------------------------------------------------------------------

class TestingApp_ : public Application {
  public:
    TestingApp_();
    void InitForTests();
    void LoadSession(const std::string &file_name);
  private:
    TestContext tc_;
};

TestingApp_::TestingApp_() {
    SetTestingFlag();
}

void TestingApp_::InitForTests() {
    Init(Vector2i(800, 600));
    GetTestContext(tc_);
}

void TestingApp_::LoadSession(const std::string &file_name) {
    ASSERT_TRUE(tc_.session_manager);
    const FilePath path = FilePath::Join(
        FilePath::Join(FilePath::GetTestDataPath(), "Sessions"), file_name);
    std::string error;
    ASSERT_TRUE(tc_.session_manager->LoadSession(path, error)) << error;
    EXPECT_TRUE(error.empty());
}

// ----------------------------------------------------------------------------
// SessionTestBase functions.
// ----------------------------------------------------------------------------

SessionTestBase::SessionTestBase() : app_(new TestingApp_) {
    app_->InitForTests();
}

SessionTestBase::~SessionTestBase() {
    // XXXX
}

void SessionTestBase::LoadSession(const std::string &file_name) {
    app_->LoadSession(file_name);
}
