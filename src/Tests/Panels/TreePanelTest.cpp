#include "Agents/ActionAgent.h"
#include "Managers/SelectionManager.h"
#include "Models/BoxModel.h"
#include "Models/HullModel.h"
#include "Models/RootModel.h"
#include "Panels/TreePanel.h"
#include "Panes/ButtonPane.h"
#include "Panes/ScrollingPane.h"
#include "Panes/SwitcherPane.h"
#include "Panes/TextPane.h"
#include "Place/ClickInfo.h"
#include "SG/ColorMap.h"
#include "SG/Search.h"
#include "Selection/SelPath.h"
#include "Selection/Selection.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Util/String.h"
#include "Widgets/GenericWidget.h"
#include "Widgets/PushButtonWidget.h"

// ----------------------------------------------------------------------------
// TreePanelTest class.
// ----------------------------------------------------------------------------

/// \ingroup Tests
class TreePanelTest : public PanelTestBase {
  protected:
    /// Model hierarchy returned by BuildModelHierarchy().
    struct Hierarchy {
        RootModelPtr root;
        HullModelPtr hull0, hull1;
        BoxModelPtr  box0, box1, box2, box3;
    };

    /// Derived ActionAgent that allows for testing of Action processing - it
    /// saves the most recent action.
    class TestActionAgent : public ActionAgent {
      public:
        /// Flags to determine whether move-up and move-down are enabled.
        bool can_move_up   = false;
        bool can_move_down = false;
        /// Last action that was applied.
        Action last_action = Action::kNone;
        virtual void Reset() override {};
        virtual void UpdateFromSessionState(
            const SessionState &state) override {};
        virtual bool CanApplyAction(Action action) const override {
            if (action == Action::kMovePrevious)
                return can_move_up;
            else if (action == Action::kMoveNext)
                return can_move_down;
            else
                return false;
        }
        virtual void ApplyAction(Action action) override {
            last_action = action;
        }
    };
    DECL_SHARED_PTR(TestActionAgent);

    TreePanelPtr        panel;
    TestActionAgentPtr  action_agent;
    SelectionManagerPtr selection_agent;

    Str scope_string;   ///< Used in TestModelRow() for scoping help.

    TreePanelTest();

    /// Creates and returns a Hierarchy of Models to use for testing.
    Hierarchy BuildModelHierarchy();

    /// Returns the number of rows displayed in the TreePanel.
    size_t GetRowCount();

    /// Tests that the indexed Model row in the TreePanel has a button for the
    /// named Model and uses the given color.
    void TestModelRow(int row_index, const Str &name, const Str &color_name);

    /// Clicks the model button in the indexed row.
    void ClickModelButton(int row_index, bool is_modified_click = false);

    /// Clicks the visibility button in the indexed row. If row < 0, this uses
    /// the visibility button in the session row.
    void ClickVisButton(int row_index);

    /// Clicks the expand/collapse button in the indexed row.
    void ClickExpButton(int row_index);

    /// Call this after changing the hierarchy or selection to update the
    /// TreePanel.
    void UpdatePanel() {
        panel->ModelsChanged();
        panel->UpdateForRenderPass("");
    }
};

TreePanelTest::TreePanelTest() : action_agent(new TestActionAgent),
                                 selection_agent(new SelectionManager) {
    GetContext().action_agent    = action_agent;
    GetContext().selection_agent = selection_agent;
    panel = InitPanel<TreePanel>("TreePanel");
}

TreePanelTest::Hierarchy TreePanelTest::BuildModelHierarchy() {
    Hierarchy hier;
    hier.root  = Model::CreateModel<RootModel>("Root");
    hier.hull0 = Model::CreateModel<HullModel>("Hull0");
    hier.hull1 = Model::CreateModel<HullModel>("Hull1");
    hier.box0  = Model::CreateModel<BoxModel>("Box0");
    hier.box1  = Model::CreateModel<BoxModel>("Box1");
    hier.box2  = Model::CreateModel<BoxModel>("Box2");
    hier.box3  = Model::CreateModel<BoxModel>("Box3");

    hier.hull0->SetOperandModels(std::vector<ModelPtr>{ hier.box0, hier.box1 });
    hier.hull1->SetOperandModels(std::vector<ModelPtr>{ hier.box2, hier.box3 });
    hier.root->AddChildModel(hier.hull0);
    hier.root->AddChildModel(hier.hull1);

    selection_agent->SetRootModel(hier.root);

    return hier;
}

size_t TreePanelTest::GetRowCount() {
    auto sp = FindTypedPane<ScrollingPane>("Scroller");
    return sp->GetContentsPane()->GetPanes().size();
}

void TreePanelTest::TestModelRow(int row_index, const Str &name,
                                 const Str &color_name){
    SCOPED_TRACE(scope_string + ": Row " + Util::ToString(row_index));
    auto row = FindPane("ModelRow_" + Util::ToString(row_index));
    ASSERT_NOT_NULL(row);
    auto but = row->FindSubPane("ModelButton");
    EXPECT_NOT_NULL(but);
    auto text = but->FindTypedSubPane<TextPane>("Text");
    ASSERT_NOT_NULL(text);
    EXPECT_EQ(name, text->GetText());
    EXPECT_EQ(SG::ColorMap::SGetColor("TreePanel" + color_name + "Color"),
              text->GetColor());
};

void TreePanelTest::ClickModelButton(int row_index, bool is_modified_click) {
    auto row = FindPane("ModelRow_" + Util::ToString(row_index));
    ASSERT_NOT_NULL(row);
    auto but = row->FindTypedSubPane<ButtonPane>("ModelButton");
    ASSERT_NOT_NULL(but);
    ClickInfo info;  // Most contents do not matter.
    info.is_modified_mode = is_modified_click;
    but->GetButton().Click(info);
}

void TreePanelTest::ClickVisButton(int row_index) {
    const Str row_name = row_index < 0 ?
        "SessionRow" : "ModelRow_" + Util::ToString(row_index);
    auto row = FindPane(row_name);
    ASSERT_NOT_NULL(row);
    auto sw  = row->FindTypedSubPane<SwitcherPane>("VisSwitcher");
    // Choose the hide or show button under the switcher
    auto but = sw->FindTypedSubPane<ButtonPane>(
        sw->GetIndex() == 1 ? "HideButton" : "ShowButton");
    ASSERT_NOT_NULL(but);
    ClickInfo info;  // Contents do not matter.
    but->GetButton().Click(info);
}

void TreePanelTest::ClickExpButton(int row_index) {
    auto row = FindPane("ModelRow_" + Util::ToString(row_index));
    ASSERT_NOT_NULL(row);
    auto sw  = row->FindTypedSubPane<SwitcherPane>("ExpSwitcher");
    // Choose the hide or show button under the switcher
    auto but = sw->FindTypedSubPane<ButtonPane>(
        sw->GetIndex() == 1 ? "CollapseButton" : "ExpandButton");
    ASSERT_NOT_NULL(but);
    ClickInfo info;  // Contents do not matter.
    but->GetButton().Click(info);
}

// ----------------------------------------------------------------------------
// TreePanelTest tests.
// ----------------------------------------------------------------------------

TEST_F(TreePanelTest, Defaults) {
    EXPECT_NULL(panel->GetFocusedPane());
    EXPECT_EQ(".", FindTypedPane<TextPane>("SessionString")->GetText());
    EXPECT_FALSE(panel->IsMovable());
    EXPECT_FALSE(panel->IsResizable());
    EXPECT_FALSE(panel->CanGripHover());
    EXPECT_NULL(panel->GetGripWidget(Point2f(0, 0)));
}

TEST_F(TreePanelTest, Show) {
    panel->SetSessionString("Session");
    EXPECT_EQ("Session", FindTypedPane<TextPane>("SessionString")->GetText());

    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());
    EXPECT_EQ(FindPane("HideButton"), panel->GetFocusedPane());
}

TEST_F(TreePanelTest, Models) {
    const auto hier = BuildModelHierarchy();

    panel->SetRootModel(hier.root);
    panel->SetStatus(Panel::Status::kVisible);

    // Call this so the TreePanel builds the Model rows.
    UpdatePanel();

    // These are in hierarchy order.
    scope_string = "Models orig";
    EXPECT_EQ(6U, GetRowCount());
    TestModelRow(0, "Hull0", "Default");
    TestModelRow(1, "Box0",  "HiddenByModel");
    TestModelRow(2, "Box1",  "HiddenByModel");
    TestModelRow(3, "Hull1", "Default");
    TestModelRow(4, "Box2",  "HiddenByModel");
    TestModelRow(5, "Box3",  "HiddenByModel");

    // Remove a model and update. The rows should be modified.
    hier.hull1->RemoveChildModel(0);
    UpdatePanel();
    scope_string = "Models after remove";
    EXPECT_EQ(5U, GetRowCount());
    TestModelRow(0, "Hull0", "Default");
    TestModelRow(1, "Box0",  "HiddenByModel");
    TestModelRow(2, "Box1",  "HiddenByModel");
    TestModelRow(3, "Hull1", "Default");
    TestModelRow(4, "Box3",  "HiddenByModel");
}

TEST_F(TreePanelTest, Select) {
    ASSERT_NOT_NULL(selection_agent);

    const auto hier = BuildModelHierarchy();

    panel->SetRootModel(hier.root);
    panel->SetStatus(Panel::Status::kVisible);

    // Select the Hull1 model.
    SelPath sel_path(hier.root, hier.hull1);
    selection_agent->ChangeModelSelection(sel_path, false);
    EXPECT_ENUM_EQ(Model::Status::kPrimary, hier.hull1->GetStatus());
    UpdatePanel();
    scope_string = "Select Hull1";
    EXPECT_EQ(6U, GetRowCount());
    TestModelRow(0, "Hull0", "Default");
    TestModelRow(1, "Box0",  "HiddenByModel");
    TestModelRow(2, "Box1",  "HiddenByModel");
    TestModelRow(3, "Hull1", "Primary");
    TestModelRow(4, "Box2",  "HiddenByModel");
    TestModelRow(5, "Box3",  "HiddenByModel");

    // Clear the selection.
    selection_agent->ChangeSelection(Selection());
    EXPECT_ENUM_EQ(Model::Status::kUnselected, hier.hull1->GetStatus());
    UpdatePanel();
    scope_string = "Clear Selection";
    EXPECT_EQ(6U, GetRowCount());
    TestModelRow(0, "Hull0", "Default");
    TestModelRow(1, "Box0",  "HiddenByModel");
    TestModelRow(2, "Box1",  "HiddenByModel");
    TestModelRow(3, "Hull1", "Default");
    TestModelRow(4, "Box2",  "HiddenByModel");
    TestModelRow(5, "Box3",  "HiddenByModel");

    // Select both children of the Hull0 model.
    selection_agent->ChangeSelection(Selection());
    sel_path = SelPath(hier.root, hier.hull0);
    sel_path.push_back(hier.box0);
    selection_agent->ChangeModelSelection(sel_path, false);
    sel_path.pop_back();
    sel_path.push_back(hier.box1);
    selection_agent->ChangeModelSelection(sel_path, true);
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   hier.box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, hier.box1->GetStatus());
    UpdatePanel();
    scope_string = "Select Box0 and Box1";
    EXPECT_EQ(6U, GetRowCount());
    TestModelRow(0, "Hull0", "HiddenByModel");
    TestModelRow(1, "Box0",  "Primary");
    TestModelRow(2, "Box1",  "Secondary");
    TestModelRow(3, "Hull1", "Default");
    TestModelRow(4, "Box2",  "HiddenByModel");
    TestModelRow(5, "Box3",  "HiddenByModel");

    // Click on the Box3 button to select it.
    ClickModelButton(5);
    EXPECT_ENUM_EQ(Model::Status::kAncestorShown,   hier.box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kAncestorShown,   hier.box1->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kDescendantShown, hier.hull1->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,         hier.box3->GetStatus());
    UpdatePanel();
    scope_string = "Click on Box3";
    EXPECT_EQ(6U, GetRowCount());
    TestModelRow(0, "Hull0", "Default");
    TestModelRow(1, "Box0",  "HiddenByModel");
    TestModelRow(2, "Box1",  "HiddenByModel");
    TestModelRow(3, "Hull1", "HiddenByModel");
    TestModelRow(4, "Box2",  "Default");
    TestModelRow(5, "Box3",  "Primary");

    // Modified-Click on the Box2 button to also select it.
    ClickModelButton(4, true);
    UpdatePanel();
    scope_string = "Mod-Click on Box2";
    EXPECT_EQ(6U, GetRowCount());
    TestModelRow(0, "Hull0", "Default");
    TestModelRow(1, "Box0",  "HiddenByModel");
    TestModelRow(2, "Box1",  "HiddenByModel");
    TestModelRow(3, "Hull1", "HiddenByModel");
    TestModelRow(4, "Box2",  "Secondary");
    TestModelRow(5, "Box3",  "Primary");
}

TEST_F(TreePanelTest, Visibility) {
    const auto hier = BuildModelHierarchy();

    panel->SetRootModel(hier.root);
    panel->SetStatus(Panel::Status::kVisible);
    UpdatePanel();

    // Select Hull1 by clicking its name.
    ClickModelButton(3);
    UpdatePanel();
    EXPECT_ENUM_EQ(Model::Status::kPrimary, hier.hull1->GetStatus());
    scope_string = "Select Hull1";
    EXPECT_EQ(6U, GetRowCount());
    TestModelRow(0, "Hull0", "Default");
    TestModelRow(1, "Box0",  "HiddenByModel");
    TestModelRow(2, "Box1",  "HiddenByModel");
    TestModelRow(3, "Hull1", "Primary");
    TestModelRow(4, "Box2",  "HiddenByModel");
    TestModelRow(5, "Box3",  "HiddenByModel");

    // Hide Hull1 - should deselect it.
    ClickVisButton(3);
    UpdatePanel();
    EXPECT_ENUM_EQ(Model::Status::kHiddenByUser, hier.hull1->GetStatus());
    scope_string = "Hide Hull1";
    EXPECT_EQ(6U, GetRowCount());
    TestModelRow(0, "Hull0", "Default");
    TestModelRow(1, "Box0",  "HiddenByModel");
    TestModelRow(2, "Box1",  "HiddenByModel");
    TestModelRow(3, "Hull1", "HiddenByUser");
    TestModelRow(4, "Box2",  "HiddenByUser");
    TestModelRow(5, "Box3",  "HiddenByUser");

    // Click the session row vis button to show all models.
    ClickVisButton(-1);
    UpdatePanel();
    EXPECT_ENUM_EQ(Model::Status::kUnselected, hier.hull1->GetStatus());
    scope_string = "Show All";
    EXPECT_EQ(6U, GetRowCount());
    TestModelRow(0, "Hull0", "Default");
    TestModelRow(1, "Box0",  "HiddenByModel");
    TestModelRow(2, "Box1",  "HiddenByModel");
    TestModelRow(3, "Hull1", "Default");
    TestModelRow(4, "Box2",  "HiddenByModel");
    TestModelRow(5, "Box3",  "HiddenByModel");

    // Click it again to hide all models.
    ClickVisButton(-1);
    UpdatePanel();
    scope_string = "Hide All";
    EXPECT_EQ(6U, GetRowCount());
    TestModelRow(0, "Hull0", "HiddenByUser");
    TestModelRow(1, "Box0",  "HiddenByUser");
    TestModelRow(2, "Box1",  "HiddenByUser");
    TestModelRow(3, "Hull1", "HiddenByUser");
    TestModelRow(4, "Box2",  "HiddenByUser");
    TestModelRow(5, "Box3",  "HiddenByUser");

    // Select Hull0 - it should become visible.
    ClickModelButton(0);
    UpdatePanel();
    scope_string = "Select Hull0 to show it";
    EXPECT_EQ(6U, GetRowCount());
    TestModelRow(0, "Hull0", "Primary");
    TestModelRow(1, "Box0",  "HiddenByModel");
    TestModelRow(2, "Box1",  "HiddenByModel");
    TestModelRow(3, "Hull1", "HiddenByUser");
    TestModelRow(4, "Box2",  "HiddenByUser");
    TestModelRow(5, "Box3",  "HiddenByUser");

    // Show Hull1.
    ClickVisButton(3);
    UpdatePanel();
    scope_string = "Show Hull1";
    EXPECT_EQ(6U, GetRowCount());
    TestModelRow(0, "Hull0", "Primary");
    TestModelRow(1, "Box0",  "HiddenByModel");
    TestModelRow(2, "Box1",  "HiddenByModel");
    TestModelRow(3, "Hull1", "Default");
    TestModelRow(4, "Box2",  "HiddenByModel");
    TestModelRow(5, "Box3",  "HiddenByModel");
}

TEST_F(TreePanelTest, ExpandCollapse) {
    const auto hier = BuildModelHierarchy();

    panel->SetRootModel(hier.root);
    panel->SetStatus(Panel::Status::kVisible);
    UpdatePanel();
    EXPECT_EQ(6U, GetRowCount());

    // Collapse Hull0.
    ClickExpButton(0);
    UpdatePanel();
    scope_string = "Collapse Hull0";
    EXPECT_EQ(4U, GetRowCount());
    TestModelRow(0, "Hull0", "Default");
    TestModelRow(1, "Hull1", "Default");
    TestModelRow(2, "Box2",  "HiddenByModel");
    TestModelRow(3, "Box3",  "HiddenByModel");

    // Collapse Hull1.
    ClickExpButton(1);
    UpdatePanel();
    scope_string = "Collapse Hull1";
    EXPECT_EQ(2U, GetRowCount());
    TestModelRow(0, "Hull0", "Default");
    TestModelRow(1, "Hull1", "Default");

    // Expand Hull0.
    ClickExpButton(0);
    UpdatePanel();
    scope_string = "Expand Hull0";
    EXPECT_EQ(4U, GetRowCount());
    TestModelRow(0, "Hull0", "Default");
    TestModelRow(1, "Box0",  "HiddenByModel");
    TestModelRow(2, "Box1",  "HiddenByModel");
    TestModelRow(3, "Hull1", "Default");
}

TEST_F(TreePanelTest, RectSelect) {
    const auto hier = BuildModelHierarchy();

    panel->SetRootModel(hier.root);
    panel->SetStatus(Panel::Status::kVisible);
    UpdatePanel();

    // Access the GenericWidget used for rectangle selection.
    auto rw = SG::FindTypedNodeUnderNode<GenericWidget>(*panel, "Widget");

    // Simulate a rectangle drag over the entire pane. This should select Hull1
    // as primary and Hull0 as secondary.
    DragTester dt(rw);
    dt.ApplyMouseDrag(Point3f(-.5f, -.5f, 0), Point3f(.5f, .5f, 0));
    UpdatePanel();
    scope_string = "Drag select all";
    EXPECT_EQ(6U, GetRowCount());
    TestModelRow(0, "Hull0", "Secondary");
    TestModelRow(1, "Box0",  "HiddenByModel");
    TestModelRow(2, "Box1",  "HiddenByModel");
    TestModelRow(3, "Hull1", "Primary");
    TestModelRow(4, "Box2",  "HiddenByModel");
    TestModelRow(5, "Box3",  "HiddenByModel");

    // Modified-drag over just Hull0 to deselect it. Pass true to
    // ApplyMouseDrag() so the end of the drag is off the widget.
    dt.SetIsModifiedMode(true);
    dt.ApplyMouseDrag(Point3f(-.2f, .48f, 0), Point3f(.6f, .6f, 0), 1, true);
    UpdatePanel();
    scope_string = "Drag deselect Hull0";
    EXPECT_EQ(6U, GetRowCount());
    TestModelRow(0, "Hull0", "Default");
    TestModelRow(1, "Box0",  "HiddenByModel");
    TestModelRow(2, "Box1",  "HiddenByModel");
    TestModelRow(3, "Hull1", "Primary");
    TestModelRow(4, "Box2",  "HiddenByModel");
    TestModelRow(5, "Box3",  "HiddenByModel");
}

TEST_F(TreePanelTest, MoveUpDown) {
    const auto hier = BuildModelHierarchy();
    panel->SetRootModel(hier.root);
    panel->SetStatus(Panel::Status::kVisible);
    UpdatePanel();

    // The TreePanel relies on the ActionAgent to tell it whether actions are
    // enabled.
    action_agent->can_move_up = action_agent->can_move_down = false;
    UpdatePanel();
    EXPECT_FALSE(IsButtonPaneEnabled("MoveUp"));
    EXPECT_FALSE(IsButtonPaneEnabled("MoveDown"));

    // Select Hull1 - MoveUp would be enabled.
    ClickModelButton(3);
    action_agent->can_move_up = true;
    UpdatePanel();
    EXPECT_TRUE(IsButtonPaneEnabled("MoveUp"));
    EXPECT_FALSE(IsButtonPaneEnabled("MoveDown"));

    // Click on the MoveUp button. They should swap enabling and the action
    // should have been applied.
    EXPECT_ENUM_EQ(Action::kNone, action_agent->last_action);
    ClickButtonPane("MoveUp");
    action_agent->can_move_up   = false;
    action_agent->can_move_down = true;
    UpdatePanel();
    EXPECT_FALSE(IsButtonPaneEnabled("MoveUp"));
    EXPECT_TRUE(IsButtonPaneEnabled("MoveDown"));
    EXPECT_ENUM_EQ(Action::kMovePrevious, action_agent->last_action);

    // Repeat with MoveDown.
    ClickButtonPane("MoveDown");
    action_agent->can_move_up   = true;
    action_agent->can_move_down = false;
    UpdatePanel();
    EXPECT_TRUE(IsButtonPaneEnabled("MoveUp"));
    EXPECT_FALSE(IsButtonPaneEnabled("MoveDown"));
    EXPECT_ENUM_EQ(Action::kMoveNext, action_agent->last_action);
}

TEST_F(TreePanelTest, Reset) {
    const auto hier = BuildModelHierarchy();
    panel->SetRootModel(hier.root);
    panel->SetStatus(Panel::Status::kVisible);
    UpdatePanel();
    EXPECT_EQ(6U, GetRowCount());

    hier.root->ClearChildModels();
    panel->Reset();
    EXPECT_EQ(0U, GetRowCount());
}
