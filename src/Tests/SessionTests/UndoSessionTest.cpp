#include "App/SceneContext.h"
#include "Commands/CommandList.h"
#include "Managers/CommandManager.h"
#include "Math/Types.h"
#include "Models/RootModel.h"
#include "SessionTests/SessionTestBase.h"
#include "Testing.h"

TEST_F(SessionTestBase, UndoSessionTest) {
    // Tests that loading a session with undo at the end works properly.
    LoadSession("Undo.mvr");

    const auto &rm = *context.scene_context->root_model;
    EXPECT_EQ(1U, rm.GetChildModelCount());
    const auto &box = *rm.GetChildModel(0);

    // The last command (translating the box) should have been undone, so it
    // should be at the default position.
    EXPECT_EQ("Box_1", box.GetName());
    EXPECT_EQ(Vector3f(0, 4, 0), box.GetTranslation());

    // It should be possible to undo (box creation) and redo (box translation).
    EXPECT_TRUE(context.command_manager->CanUndo());
    EXPECT_TRUE(context.command_manager->CanRedo());

    // Redo and make sure the translation (by 14 in X) is applied.
    context.command_manager->Redo();
    EXPECT_EQ(Vector3f(14, 4, 0), box.GetTranslation());
}
