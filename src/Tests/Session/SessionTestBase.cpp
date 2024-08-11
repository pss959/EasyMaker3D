//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tests/Session/SessionTestBase.h"

#include "App/Application.h"
#include "Managers/SceneContext.h"
#include "Managers/SelectionManager.h"
#include "Managers/SessionManager.h"
#include "SG/Search.h"
#include "Tests/Testing.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/General.h"
#include "Util/Tuning.h"

// ----------------------------------------------------------------------------
// SessionTestBase::TestingApp_ class.
// ----------------------------------------------------------------------------

/// Derived Application class that provides functions for testing via the
/// Application's protected interface.
///
/// \ingroup Tests
class SessionTestBase::TestingApp_ : public Application {
  public:
    void InitForTests();

    // Make this available to SessionTestBase.
    using Application::GetContext;
};

void SessionTestBase::TestingApp_::InitForTests() {
    Application::Options options;
    options.window_size.Set(800, 600);
    options.connect_vr = false;  // Bypasses VR system init.
    Init(options);

    // Make sure the TestContext has what it needs.
    ASSERT(GetContext().scene_context);
    ASSERT(GetContext().scene_context->frustum);
}

// ----------------------------------------------------------------------------
// SessionTestBase class.
// ----------------------------------------------------------------------------

SessionTestBase::SessionTestBase() : app_(new TestingApp_) {
    Util::app_type = Util::AppType::kUnitTest;
}

SessionTestBase::~SessionTestBase() {
}

void SessionTestBase::SetUp() {
    app_->InitForTests();

    // Copy Application::Context locally for convenience in derived classes.
    context = app_->GetContext();
}

bool SessionTestBase::LoadSession(const Str &file_name) {
    ASSERT(context.session_manager);
    const FilePath path = FilePath::Join(
        FilePath::Join(FilePath::GetTestDataPath(), "Sessions"),
        file_name + TK::kSessionFileExtension);
    Str error;
    EXPECT_TRUE(context.session_manager->LoadSession(path, error)) << error;
    return error.empty();
}

void SessionTestBase::SelectModel(const Model &model) {
    const auto path =
        SG::FindNodePathUnderNode(context.scene_context->root_model, model);
    context.selection_manager->ChangeSelection(Selection(path));
}
