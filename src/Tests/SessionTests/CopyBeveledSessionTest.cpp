#include "SessionTests/SessionTestBase.h"
#include "Tests/Testing.h"

TEST_F(SessionTestBase, CopyBeveledSessionTest) {
    // Tests a bug copying and pasting a BeveledModel.
    LoadSession("CopyBeveled.mvr");
}

