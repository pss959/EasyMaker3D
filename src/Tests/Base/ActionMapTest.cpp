#include "Base/ActionMap.h"
#include "Tests/Testing.h"
#include "Util/General.h"

TEST(ActionMapTest, GetCategoryForAction) {
    ActionMap am;
    EXPECT_ENUM_EQ(ActionCategory::kBasics,
                   am.GetCategoryForAction(Action::kCopy));
    EXPECT_ENUM_EQ(ActionCategory::kSession,
                   am.GetCategoryForAction(Action::kOpenInfoPanel));
    EXPECT_ENUM_EQ(ActionCategory::kSpecialized,
                   am.GetCategoryForAction(Action::kToggleSpecializedTool));
    EXPECT_ENUM_EQ(ActionCategory::kViewing,
                   am.GetCategoryForAction(Action::kToggleShowEdges));
    EXPECT_ENUM_EQ(ActionCategory::kNone,
                   am.GetCategoryForAction(Action::kNone));
}

TEST(ActionMapTest, GetActionsInCategory) {
    ActionMap am;
    {
        const auto &actions = am.GetActionsInCategory(ActionCategory::kNone);
        EXPECT_EQ(1U, actions.size());
        EXPECT_ENUM_EQ(Action::kNone, actions[0]);
    }
    {
        const auto &actions = am.GetActionsInCategory(ActionCategory::kBasics);
        EXPECT_EQ(9U, actions.size());
        EXPECT_TRUE(Util::Contains(actions, Action::kCopy));
        EXPECT_TRUE(Util::Contains(actions, Action::kCut));
        EXPECT_TRUE(Util::Contains(actions, Action::kDelete));
        EXPECT_TRUE(Util::Contains(actions, Action::kDuplicate));
        EXPECT_TRUE(Util::Contains(actions, Action::kPaste));
        EXPECT_TRUE(Util::Contains(actions, Action::kPasteInto));
        EXPECT_TRUE(Util::Contains(actions, Action::kQuit));
        EXPECT_TRUE(Util::Contains(actions, Action::kRedo));
        EXPECT_TRUE(Util::Contains(actions, Action::kUndo));
    }
}
