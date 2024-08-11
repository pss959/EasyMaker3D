//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Managers/PanelManager.h"
#include "Panels/InfoPanel.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing2.h"
#include "Util/Assert.h"

/// \ingroup Tests
class PanelManagerTest : public SceneTestBase {};

TEST_F(PanelManagerTest, GetPanel) {
    const Str contents = R"(
  children: [
    <"nodes/templates/RadialMenu.emd">, # Required for RadialMenuPanel
    <"nodes/Panels.emd">
  ]
)";

    // Read a Scene that sets up all known Panels.
    auto scene = ReadRealScene(contents);

    PanelManager pm;

    TEST_ASSERT(pm.GetPanel("ActionPanel"), "No panel");

    Panel::ContextPtr context(new Panel::Context);  // Contents do not matter.
    pm.FindAllPanels(*scene, context);
    EXPECT_NOT_NULL(pm.GetPanel("ActionPanel"));
    EXPECT_NOT_NULL(pm.GetPanel("BevelToolPanel"));
    EXPECT_NOT_NULL(pm.GetPanel("CSGToolPanel"));
    EXPECT_NOT_NULL(pm.GetPanel("DialogPanel"));
    EXPECT_NOT_NULL(pm.GetPanel("ExtrudedToolPanel"));
    EXPECT_NOT_NULL(pm.GetPanel("FilePanel"));
    EXPECT_NOT_NULL(pm.GetPanel("HelpPanel"));
    EXPECT_NOT_NULL(pm.GetPanel("ImportToolPanel"));
    EXPECT_NOT_NULL(pm.GetPanel("InfoPanel"));
    EXPECT_NOT_NULL(pm.GetPanel("KeyboardPanel"));
    EXPECT_NOT_NULL(pm.GetPanel("NameToolPanel"));
    EXPECT_NOT_NULL(pm.GetPanel("RadialMenuPanel"));
    EXPECT_NOT_NULL(pm.GetPanel("RevSurfToolPanel"));
    EXPECT_NOT_NULL(pm.GetPanel("SessionPanel"));
    EXPECT_NOT_NULL(pm.GetPanel("SettingsPanel"));
    EXPECT_NOT_NULL(pm.GetPanel("TaperToolPanel"));
    EXPECT_NOT_NULL(pm.GetPanel("TextToolPanel"));

    // Check GetTypedPanel() also.
    EXPECT_NOT_NULL(pm.GetTypedPanel<InfoPanel>("InfoPanel"));
    TEST_ASSERT(pm.GetTypedPanel<InfoPanel>("HelpPanel"), "panel");

    pm.Reset();
    TEST_ASSERT(pm.GetPanel("ActionPanel"), "No panel");
}
