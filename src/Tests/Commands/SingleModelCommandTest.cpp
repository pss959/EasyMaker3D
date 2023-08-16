#include "Commands/ChangeNameCommand.h"
#include "Math/Types.h"
#include "Selection/Selection.h"
#include "Tests/SelectionTestBase.h"
#include "Tests/Testing.h"
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
    TEST_THROW(cnc->SetFromSelection(sel), AssertException, "GetCount() == 1U");

    // Create a selection with one Model.
    sel.Add(BuildSelPath(ModelVec{ root, par0 }));
    cnc->SetFromSelection(sel);
    EXPECT_EQ(par0->GetName(), cnc->GetModelName());

    // More than one Model is an error..
    sel.Add(BuildSelPath(ModelVec{ root, par1, box1 }));
    TEST_THROW(cnc->SetFromSelection(sel), AssertException, "GetCount() == 1U");
}
