#include "SessionTests/SessionTestBase.h"

#include "Managers/SessionManager.h"
#include "Testing.h"
#include "Util/FilePath.h"

SessionTestBase::SessionTestBase() {
    ASSERT(context.session_manager);
}

bool SessionTestBase::LoadSession(const std::string &file_name) {
    const FilePath path = FilePath::Join(
        FilePath::Join(FilePath::GetTestDataPath(), "Sessions"), file_name);
    std::string error;
    EXPECT_TRUE(context.session_manager->LoadSession(path, error)) << error;
    return error.empty();
}
