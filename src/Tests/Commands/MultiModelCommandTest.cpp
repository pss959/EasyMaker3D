//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/DeleteCommand.h"
#include "Math/Types.h"
#include "Selection/Selection.h"
#include "Tests/SelectionTestBase.h"
#include "Tests/Testing2.h"
#include "Util/Assert.h"

/// \ingroup Tests
class MultiModelCommandTest : public SelectionTestBase {};

// ----------------------------------------------------------------------------
// Because MultiModelCommand is abstract, these tests use the derived concrete
// DeleteCommand class.
// ----------------------------------------------------------------------------

TEST_F(MultiModelCommandTest, Defaults) {
    auto dc = Command::CreateCommand<DeleteCommand>();
    EXPECT_TRUE(dc->GetModelNames().empty());
}

TEST_F(MultiModelCommandTest, SetFromSelection) {
    auto dc = Command::CreateCommand<DeleteCommand>();

    Selection sel;
    TEST_ASSERT(dc->SetFromSelection(sel), "HasAny");

    // Create a selection with one Model.
    sel.Add(BuildSelPath(ModelVec{ root, par0 }));
    dc->SetFromSelection(sel);
    EXPECT_EQ(1U,              dc->GetModelNames().size());
    EXPECT_EQ(par0->GetName(), dc->GetModelNames()[0]);

    // Two Models.
    sel.Add(BuildSelPath(ModelVec{ root, par1, box1 }));
    dc->SetFromSelection(sel);
    EXPECT_EQ(2U,              dc->GetModelNames().size());
    EXPECT_EQ(par0->GetName(), dc->GetModelNames()[0]);
    EXPECT_EQ(box1->GetName(), dc->GetModelNames()[1]);
}
