#include "SessionTests/SessionTestBase.h"
#include "Tests/Testing.h"

TEST_F(SessionTestBase, CopyBeveledSessionTest) {
    // Tests a bug copying a BeveledModel.
    LoadSession("CopyBeveled.mvr");

}
