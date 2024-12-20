//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Managers/SceneContext.h"
#include "Models/RootModel.h"
#include "Tests/Session/SessionTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class PasteSessionTest : public SessionTestBase {};

TEST_F(PasteSessionTest, Paste) {
    // This session creates a CylinderModel, copies it, and pastes it.
    LoadSession("PasteCyl");
    const auto &rm = *context.scene_context->root_model;
    EXPECT_EQ(2U, rm.GetChildModelCount());
    const auto &orig = *rm.GetChildModel(0);
    const auto &copy = *rm.GetChildModel(1);

    EXPECT_EQ("Cylinder_1",   orig.GetName());
    EXPECT_EQ("Cylinder_1_A", copy.GetName());
}

TEST_F(PasteSessionTest, PasteRenamed) {
    // This session creates a CylinderModel, renames it, copies it, and pastes
    // it.

    LoadSession("PasteRenamed");
    const auto &rm = *context.scene_context->root_model;
    EXPECT_EQ(2U, rm.GetChildModelCount());
    const auto &orig = *rm.GetChildModel(0);
    const auto &copy = *rm.GetChildModel(1);

    EXPECT_EQ("Renamed",   orig.GetName());
    EXPECT_EQ("Renamed_A", copy.GetName());
}
