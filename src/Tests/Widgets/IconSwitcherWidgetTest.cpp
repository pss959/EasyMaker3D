#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/Assert.h"
#include "Widgets/IconSwitcherWidget.h"

/// \ingroup Tests
class IconSwitcherWidgetTest : public SceneTestBase {};

TEST_F(IconSwitcherWidgetTest, Defaults) {
    auto isw = CreateObject<IconSwitcherWidget>();
    EXPECT_EQ(-1, isw->GetIndex());
}

TEST_F(IconSwitcherWidgetTest, Switch) {
    // Create an instance with 3 children.
    const Str input = R"(
IconSwitcherWidget {
  children: [
    IconWidget "IW0" {},
    IconWidget "IW1" {},
    IconWidget "IW2" {},
  ]
}
)";
    auto isw = ParseTypedObject<IconSwitcherWidget>(input);
    EXPECT_EQ(3U, isw->GetChildren().size());
    EXPECT_EQ(-1, isw->GetIndex());
    EXPECT_FALSE(isw->GetChildren()[0]->IsEnabled());
    EXPECT_FALSE(isw->GetChildren()[1]->IsEnabled());
    EXPECT_FALSE(isw->GetChildren()[2]->IsEnabled());

    isw->SetIndex(1);
    EXPECT_EQ(1, isw->GetIndex());
    EXPECT_FALSE(isw->GetChildren()[0]->IsEnabled());
    EXPECT_TRUE(isw->GetChildren()[1]->IsEnabled());
    EXPECT_FALSE(isw->GetChildren()[2]->IsEnabled());

    isw->SetIndexByName("IW2");
    EXPECT_EQ(2, isw->GetIndex());
    EXPECT_FALSE(isw->GetChildren()[0]->IsEnabled());
    EXPECT_FALSE(isw->GetChildren()[1]->IsEnabled());
    EXPECT_TRUE(isw->GetChildren()[2]->IsEnabled());

    isw->SetIndex(6);
    EXPECT_EQ(-1, isw->GetIndex());
    EXPECT_FALSE(isw->GetChildren()[0]->IsEnabled());
    EXPECT_FALSE(isw->GetChildren()[1]->IsEnabled());
    EXPECT_FALSE(isw->GetChildren()[2]->IsEnabled());

    TEST_THROW(isw->SetIndexByName("NoSuchChild"),
               AssertException, "No child with name");
}

TEST_F(IconSwitcherWidgetTest, FitIntoCube) {
    // Create an instance with children of different sizes.
    const Str input = R"(
IconSwitcherWidget {
  children: [
    IconWidget "IW0" { shapes: [ Box { size: 1 2 3 } ] },
    IconWidget "IW1" { shapes: [ Box { size: 2 1 6 } ] },
  ]
}
)";
    auto isw = ParseTypedObject<IconSwitcherWidget>(input);
    isw->SetIndex(1);
    isw->FitIntoCube(12, Point3f(10, 10, 10));

    // IconSwitcherWidget should be translated to center.
    EXPECT_EQ(Vector3f(1, 1, 1),    isw->GetScale());
    EXPECT_EQ(Vector3f(10, 10, 10), isw->GetTranslation());

    // Children should be scaled to fit.
    EXPECT_EQ(Vector3f(4, 4, 4), isw->GetChildren()[0]->GetScale());
    EXPECT_EQ(Vector3f(2, 2, 2), isw->GetChildren()[1]->GetScale());
    EXPECT_EQ(Vector3f(0, 0, 0), isw->GetChildren()[0]->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0), isw->GetChildren()[1]->GetTranslation());

    // Index should not have changed.
    EXPECT_EQ(1, isw->GetIndex());
}
