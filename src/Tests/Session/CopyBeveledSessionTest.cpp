#include "Tests/Session/SessionTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class CopyBeveledSessionTest : public SessionTestBase {};

TEST_F(CopyBeveledSessionTest, CopyPaste) {
    // Tests a bug copying and pasting a BeveledModel.
    LoadSession("CopyBeveled");
}

