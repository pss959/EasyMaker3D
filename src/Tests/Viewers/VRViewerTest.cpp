//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Base/Event.h"
#include "SG/Scene.h"
#include "SG/VRCamera.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Viewers/FakeRenderer.h"
#include "Tests/Viewers/FakeVRSystem.h"
#include "Viewers/IRenderer.h"
#include "Viewers/VRViewer.h"

/// \ingroup Tests
class VRViewerTest : public SceneTestBase {};

TEST_F(VRViewerTest, RenderAndEmit) {
    // VRViewer is a thin wrapper around an IVRSystem. Use a FakeVRSystem and
    // make sure it is called properly.

    FakeVRSystemPtr vrs(new FakeVRSystem);
    FakeRenderer    renderer;
    auto            scene = CreateObject<SG::Scene>();
    auto            cam   = ParseTypedObject<SG::VRCamera>(
        "VRCamera { base_position: 1 2 3 }");

    VRViewer viewer(vrs);

    // Test rendering.
    EXPECT_EQ(0U, renderer.render_count);
    viewer.Render(*scene, renderer);
    EXPECT_EQ(1U, renderer.render_count);

    // Test emitting events.
    EXPECT_EQ(0U, vrs->emit_count);
    std::vector<Event> events;
    viewer.EmitEvents(events);
    EXPECT_EQ(1U, vrs->emit_count);

    // Should have no effect.
    viewer.FlushPendingEvents();
}
