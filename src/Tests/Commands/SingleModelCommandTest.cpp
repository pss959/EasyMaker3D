//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeNameCommand.h"
#include "Math/Types.h"
#include "Selection/Selection.h"
#include "Tests/SelectionTestBase.h"
#include "Tests/Testing2.h"
#include "Util/Assert.h"

/// \ingroup Tests
class SingleModelCommandTest : public SelectionTestBase {};

// ----------------------------------------------------------------------------
// Because SingleModelCommand is abstract, these tests use the derived concrete
// ChangeNameCommand class.
// ----------------------------------------------------------------------------

TEST_F(SingleModelCommandTest, Defaults) {
    auto cnc = Command::CreateCommand<ChangeNameCommand>();
    EXPECT_TRUE(cnc->GetModelName().empty());
}

TEST_F(SingleModelCommandTest, SetFromSelection) {
    auto cnc = Command::CreateCommand<ChangeNameCommand>();

    Selection sel;
    TEST_ASSERT(cnc->SetFromSelection(sel), "GetCount() == 1U");

    // Create a selection with one Model.
    sel.Add(BuildSelPath(ModelVec{ root, par0 }));
    cnc->SetFromSelection(sel);
    EXPECT_EQ(par0->GetName(), cnc->GetModelName());

    // More than one Model is an error..
    sel.Add(BuildSelPath(ModelVec{ root, par1, box1 }));
    TEST_ASSERT(cnc->SetFromSelection(sel), "GetCount() == 1U");
}
