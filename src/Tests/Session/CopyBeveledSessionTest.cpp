//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tests/Session/SessionTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class CopyBeveledSessionTest : public SessionTestBase {};

TEST_F(CopyBeveledSessionTest, CopyPaste) {
    // Tests a bug copying and pasting a BeveledModel.
    LoadSession("CopyBeveled");
}

