#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Widgets/IconWidget.h"

class IconWidgetTest : public SceneTestBase {};

TEST_F(IconWidgetTest, Defaults) {
    auto iw = CreateObject<IconWidget>();

    EXPECT_EQ(Action::kNone, iw->GetAction());
    EXPECT_TRUE(iw->GetImportPath().empty());
    EXPECT_FALSE(iw->IsToggle());
}

TEST_F(IconWidgetTest, Fields) {
    auto iw = ParseObject<IconWidget>(
        "IconWidget { action: \"kDelete\", import_path: \"DeleteIcon.off\" }");
    EXPECT_EQ(Action::kDelete,  iw->GetAction());
    EXPECT_EQ("DeleteIcon.off", iw->GetImportPath());
    EXPECT_EQ(1U,               iw->GetShapes().size());
}

TEST_F(IconWidgetTest, ActiveHovering) {
    // SupportsActiveHovering() should be true only for toggles Use hover
    // scaling to check.
    auto iw = ParseObject<IconWidget>("IconWidget { hover_scale: 2 2 2 }");

    // Need to set up Ion for hovering.
    auto scene = ReadScene(ReadDataFile("RealScene.emd"));
    scene->GetRootNode()->AddChild(iw);

    iw->SetActive(true);
    iw->StartHovering();
    EXPECT_TRUE(iw->IsHovering());
    EXPECT_EQ(Vector3f(1, 1, 1), iw->GetScale());
    iw->StopHovering();
    EXPECT_FALSE(iw->IsHovering());
    EXPECT_EQ(Vector3f(1, 1, 1), iw->GetScale());

    // Try with a toggle: scale should change.
    iw->SetIsToggle(true);
    iw->StartHovering();
    EXPECT_TRUE(iw->IsHovering());
    EXPECT_EQ(Vector3f(2, 2, 2), iw->GetScale());
    iw->StopHovering();
    EXPECT_FALSE(iw->IsHovering());
    EXPECT_EQ(Vector3f(1, 1, 1), iw->GetScale());
}
