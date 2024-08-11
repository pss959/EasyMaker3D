//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/CommandList.h"
#include "Managers/CommandManager.h"
#include "Managers/SceneContext.h"
#include "Models/RootModel.h"
#include "Tests/Session/SessionTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class EmptySessionTest : public SessionTestBase {};

TEST_F(EmptySessionTest, Load) {
    EXPECT_TRUE(LoadSession("Empty"));

    EXPECT_TRUE(context.scene_context);
    EXPECT_TRUE(context.scene_context->root_model);
    EXPECT_EQ(0U, context.scene_context->root_model->GetChildModelCount());
    EXPECT_EQ(0U, context.command_manager->GetCommandList()->GetCommandCount());
}
