//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Base/Event.h"
#include "Handlers/InspectorHandler.h"
#include "Items/Inspector.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing2.h"
#include "Util/Assert.h"

/// \ingroup Tests
class InspectorHandlerTest : public SceneTestBase {
  protected:
    InspectorPtr insp;
    /// The constructor sets up an Inspector instance.
    InspectorHandlerTest();
};

InspectorHandlerTest::InspectorHandlerTest() {
    const Str contents = R"(children: [<"nodes/Inspector.emd">])";
    insp = ReadRealNode<Inspector>(contents, "Inspector");
}

TEST_F(InspectorHandlerTest, IsEnabled) {
    InspectorHandler ih;

    // The InspectorHandler needs to be enabled and have an enabled Inspector
    // to be considered enabled.
    EXPECT_FALSE(ih.IsEnabled());

    ih.SetInspector(insp);
    EXPECT_FALSE(ih.IsEnabled());

    insp->SetEnabled(true);
    EXPECT_TRUE(ih.IsEnabled());

    ih.SetEnabled(false);
    EXPECT_FALSE(ih.IsEnabled());

    ih.SetEnabled(true);
    insp->SetEnabled(false);
    EXPECT_FALSE(ih.IsEnabled());

    insp->SetEnabled(true);
    EXPECT_TRUE(ih.IsEnabled());
}

TEST_F(InspectorHandlerTest, Events) {
    Event event;

    InspectorHandler ih;

    // Cannot handle events without an Inspector installed.
    TEST_ASSERT(ih.HandleEvent(event), "inspector");
    ih.SetInspector(insp);

    // Activate the Inspector with a Box node.
    auto node = ParseTypedObject<SG::Node>("Node { shapes: [ Box {} ] }");
    insp->Activate(node, ControllerPtr());

    // The InspectorHandler traps all events while it is active, regardless of
    // whether they affect the Inspector.
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, ih.HandleEvent(event));

    // Valuator event should change scale.
    EXPECT_EQ(1, insp->GetCurrentScale());
    event.flags.Set(Event::Flag::kPosition1D);
    event.position1D = 1;
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, ih.HandleEvent(event));
    EXPECT_CLOSE(1.06f, insp->GetCurrentScale());

    // Mouse motion to change rotation.
    EXPECT_EQ(Rotationf::Identity(), insp->GetCurrentRotation());
    event.device = Event::Device::kMouse;
    event.flags.Reset(Event::Flag::kPosition1D);
    event.flags.Set(Event::Flag::kPosition2D);
    event.position2D.Set(.75f, .5f);  // Motion only to the right.
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, ih.HandleEvent(event));
    EXPECT_ROTS_CLOSE(BuildRotation(0, 1, 0, -90), insp->GetCurrentRotation());

    // Any key or button press should deactivate.
    EXPECT_TRUE(ih.IsEnabled());
    event = Event();
    event.flags.Set(Event::Flag::kButtonPress);
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, ih.HandleEvent(event));
    EXPECT_FALSE(ih.IsEnabled());
    insp->Activate(node, ControllerPtr());
    EXPECT_TRUE(ih.IsEnabled());
    event = Event();
    event.flags.Set(Event::Flag::kKeyPress);
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, ih.HandleEvent(event));
    EXPECT_FALSE(ih.IsEnabled());
}
