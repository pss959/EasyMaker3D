#include "Commands/CommandList.h"
#include "Managers/CommandManager.h"
#include "Managers/SceneContext.h"
#include "Math/Types.h"
#include "Models/RootModel.h"
#include "Tests/Session/SessionTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class UndoSessionTest : public SessionTestBase {};

TEST_F(UndoSessionTest, UndoRedo) {
    // Tests that loading a session with undo at the end works properly.
    LoadSession("Undo");

    const auto &rm = *context.scene_context->root_model;
    EXPECT_EQ(1U, rm.GetChildModelCount());
    const auto &box = *rm.GetChildModel(0);

    // The last command (translating the box) should have been undone, so it
    // should be at the default position.
    EXPECT_EQ("Box_1", box.GetName());
    EXPECT_EQ(Vector3f(0, MS, 0), box.GetTranslation());

    // It should be possible to undo (box creation) and redo (box translation).
    EXPECT_TRUE(context.command_manager->CanUndo());
    EXPECT_TRUE(context.command_manager->CanRedo());

    // Redo and make sure the translation (by 14 in X) is applied.
    context.command_manager->Redo();
    EXPECT_EQ(Vector3f(14, MS, 0), box.GetTranslation());
}
