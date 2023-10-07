#include "Selection/Selection.h"
#include "Managers/SelectionManager.h"
#include "Place/ClickInfo.h"
#include "Tests/SelectionTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class SelectionManagerTest : public SelectionTestBase {
  protected:
    SelectionManager sm;

    SelectionManagerTest() { sm.SetRootModel(root); }

    /// Convenience that returns the ith selected path as a string.
    Str GetPathString(size_t i) {
        return sm.GetSelection().GetPaths()[i].ToString();
    }

    using SMDirection = SelectionManager::Direction;  /// Shorthand.
    using SMOperation = SelectionManager::Operation;  /// Shorthand.
};

TEST_F(SelectionManagerTest, Defaults) {
    EXPECT_FALSE(sm.GetSelection().HasAny());
    EXPECT_EQ(0U, sm.GetSelection().GetCount());
    EXPECT_FALSE(sm.CanSelectInDirection(SMDirection::kParent));
    EXPECT_FALSE(sm.CanSelectInDirection(SMDirection::kFirstChild));
    EXPECT_FALSE(sm.CanSelectInDirection(SMDirection::kPreviousSibling));
    EXPECT_FALSE(sm.CanSelectInDirection(SMDirection::kNextSibling));
}

TEST_F(SelectionManagerTest, ChangeSelection) {
    Selection sel;

    SelPath ppath0 = BuildSelPath(ModelVec{ root, par0 });
    SelPath ppath1 = BuildSelPath(ModelVec{ root, par1 });
    SelPath bpath1 = BuildSelPath(ModelVec{ root, par1, box1 });

    sel.Add(ppath0);
    sm.ChangeSelection(sel);
    EXPECT_TRUE(sm.GetSelection().HasAny());
    EXPECT_EQ(1U, sm.GetSelection().GetCount());
    EXPECT_EQ("<ModelRoot/Par0>", GetPathString(0));

    sm.ChangeModelSelection(ppath0, true);  // Should deselect.
    EXPECT_FALSE(sm.GetSelection().HasAny());
    EXPECT_EQ(0U, sm.GetSelection().GetCount());

    sm.ChangeModelSelection(ppath0, true);  // Should reselect.
    EXPECT_TRUE(sm.GetSelection().HasAny());
    EXPECT_EQ(1U, sm.GetSelection().GetCount());
    EXPECT_EQ("<ModelRoot/Par0>", GetPathString(0));

    sm.ChangeModelSelection(ppath1, true);  // Should add to selection.
    EXPECT_TRUE(sm.GetSelection().HasAny());
    EXPECT_EQ(2U, sm.GetSelection().GetCount());
    EXPECT_EQ("<ModelRoot/Par0>", GetPathString(0));
    EXPECT_EQ("<ModelRoot/Par1>", GetPathString(1));

    sm.ChangeModelSelection(ppath0, true);  // Should deselect.
    EXPECT_TRUE(sm.GetSelection().HasAny());
    EXPECT_EQ(1U, sm.GetSelection().GetCount());
    EXPECT_EQ("<ModelRoot/Par1>", GetPathString(0));

    sm.ChangeModelSelection(ppath0, false);  // Should change selection to this.
    EXPECT_TRUE(sm.GetSelection().HasAny());
    EXPECT_EQ(1U, sm.GetSelection().GetCount());
    EXPECT_EQ("<ModelRoot/Par0>", GetPathString(0));

    // Select a child Model and then SelectAll() - it should deselect the child
    // and select its parent (first) when selecting all top-level Models.
    sm.ChangeModelSelection(bpath1, false);
    EXPECT_TRUE(sm.GetSelection().HasAny());
    EXPECT_EQ(1U, sm.GetSelection().GetCount());
    EXPECT_EQ("<ModelRoot/Par1/Box1>", GetPathString(0));
    sm.SelectAll();
    EXPECT_EQ(2U, sm.GetSelection().GetCount());
    EXPECT_EQ("<ModelRoot/Par1>", GetPathString(0));
    EXPECT_EQ("<ModelRoot/Par0>", GetPathString(1));

    sm.DeselectAll();
    EXPECT_FALSE(sm.GetSelection().HasAny());
    EXPECT_EQ(0U, sm.GetSelection().GetCount());

    // Reset should also deselect.
    sm.SelectAll();
    sm.Reset();
    EXPECT_FALSE(sm.GetSelection().HasAny());
    EXPECT_EQ(0U, sm.GetSelection().GetCount());
}

TEST_F(SelectionManagerTest, Directions) {
    SelPath ppath0 = BuildSelPath(ModelVec{ root, par0 });
    SelPath ppath1 = BuildSelPath(ModelVec{ root, par1 });
    SelPath bpath0 = BuildSelPath(ModelVec{ root, par0, box0 });

    // Select one parent.
    sm.ChangeModelSelection(ppath0, false);
    EXPECT_EQ(1U, sm.GetSelection().GetCount());
    EXPECT_EQ("<ModelRoot/Par0>", GetPathString(0));
    EXPECT_FALSE(sm.CanSelectInDirection(SMDirection::kParent));
    EXPECT_TRUE(sm.CanSelectInDirection(SMDirection::kFirstChild));
    EXPECT_FALSE(sm.CanSelectInDirection(SMDirection::kPreviousSibling));
    EXPECT_FALSE(sm.CanSelectInDirection(SMDirection::kNextSibling));

    // Select its first child (box0).
    sm.SelectInDirection(SMDirection::kFirstChild);
    EXPECT_EQ(1U, sm.GetSelection().GetCount());
    EXPECT_EQ("<ModelRoot/Par0/Box0>", GetPathString(0));
    EXPECT_TRUE(sm.CanSelectInDirection(SMDirection::kParent));
    EXPECT_FALSE(sm.CanSelectInDirection(SMDirection::kFirstChild));
    EXPECT_TRUE(sm.CanSelectInDirection(SMDirection::kPreviousSibling));
    EXPECT_TRUE(sm.CanSelectInDirection(SMDirection::kNextSibling));

    // Select its next sibling (box1).
    sm.SelectInDirection(SMDirection::kNextSibling);
    EXPECT_EQ(1U, sm.GetSelection().GetCount());
    EXPECT_EQ("<ModelRoot/Par0/Box1>", GetPathString(0));
    EXPECT_TRUE(sm.CanSelectInDirection(SMDirection::kParent));
    EXPECT_FALSE(sm.CanSelectInDirection(SMDirection::kFirstChild));
    EXPECT_TRUE(sm.CanSelectInDirection(SMDirection::kPreviousSibling));
    EXPECT_TRUE(sm.CanSelectInDirection(SMDirection::kNextSibling));

    // Select its next sibling, wrapping (box0).
    sm.SelectInDirection(SMDirection::kNextSibling);
    EXPECT_EQ(1U, sm.GetSelection().GetCount());
    EXPECT_EQ("<ModelRoot/Par0/Box0>", GetPathString(0));
    EXPECT_TRUE(sm.CanSelectInDirection(SMDirection::kParent));
    EXPECT_FALSE(sm.CanSelectInDirection(SMDirection::kFirstChild));
    EXPECT_TRUE(sm.CanSelectInDirection(SMDirection::kPreviousSibling));
    EXPECT_TRUE(sm.CanSelectInDirection(SMDirection::kNextSibling));

    // Select its previous sibling, wrapping (box1).
    sm.SelectInDirection(SMDirection::kPreviousSibling);
    EXPECT_EQ(1U, sm.GetSelection().GetCount());
    EXPECT_EQ("<ModelRoot/Par0/Box1>", GetPathString(0));
    EXPECT_TRUE(sm.CanSelectInDirection(SMDirection::kParent));
    EXPECT_FALSE(sm.CanSelectInDirection(SMDirection::kFirstChild));
    EXPECT_TRUE(sm.CanSelectInDirection(SMDirection::kPreviousSibling));
    EXPECT_TRUE(sm.CanSelectInDirection(SMDirection::kNextSibling));

    // Select its previous sibling (box0).
    sm.SelectInDirection(SMDirection::kPreviousSibling);
    EXPECT_EQ(1U, sm.GetSelection().GetCount());
    EXPECT_EQ("<ModelRoot/Par0/Box0>", GetPathString(0));
    EXPECT_TRUE(sm.CanSelectInDirection(SMDirection::kParent));
    EXPECT_FALSE(sm.CanSelectInDirection(SMDirection::kFirstChild));
    EXPECT_TRUE(sm.CanSelectInDirection(SMDirection::kPreviousSibling));
    EXPECT_TRUE(sm.CanSelectInDirection(SMDirection::kNextSibling));

    // Select its parent (par0).
    sm.SelectInDirection(SMDirection::kParent);
    EXPECT_EQ(1U, sm.GetSelection().GetCount());
    EXPECT_EQ("<ModelRoot/Par0>", GetPathString(0));
    EXPECT_FALSE(sm.CanSelectInDirection(SMDirection::kParent));
    EXPECT_TRUE(sm.CanSelectInDirection(SMDirection::kFirstChild));
    EXPECT_FALSE(sm.CanSelectInDirection(SMDirection::kPreviousSibling));
    EXPECT_FALSE(sm.CanSelectInDirection(SMDirection::kNextSibling));
}

TEST_F(SelectionManagerTest, Click) {
    sm.AttachClickToModel(*par0);
    EXPECT_FALSE(par0->IsSelected());

    ClickInfo info;
    info.hit.path = BuildSelPath(ModelVec{ root, par0 });
    info.is_modified_mode = false;

    par0->Click(info);
    EXPECT_TRUE(par0->IsSelected());

    info.is_modified_mode = true;  // Should toggle.
    par0->Click(info);
    EXPECT_FALSE(par0->IsSelected());
}

TEST_F(SelectionManagerTest, Clean) {
    SelPath ppath0 = BuildSelPath(ModelVec{ root, par0 });
    SelPath ppath1 = BuildSelPath(ModelVec{ root, par1 });
    SelPath bpath0 = BuildSelPath(ModelVec{ root, par0, box0 });

    Selection sel;

    // Descendent should not be selected if ancestor is (reverse order).
    sel.Add(bpath0);
    sel.Add(ppath0);
    sm.ChangeSelection(sel);
    EXPECT_EQ(1U, sm.GetSelection().GetCount());
    EXPECT_EQ("<ModelRoot/Par0>", GetPathString(0));

    // Ancestor should not be selected if descendent is (reverse order).
    sel.Clear();
    sel.Add(ppath0);
    sel.Add(bpath0);
    sm.ChangeSelection(sel);
    EXPECT_EQ(1U, sm.GetSelection().GetCount());
    EXPECT_EQ("<ModelRoot/Par0/Box0>", GetPathString(0));

    // Duplicate should not be selected.
    sel.Clear();
    sel.Add(ppath0);
    sel.Add(ppath0);
    sm.ChangeSelection(sel);
    EXPECT_EQ(1U, sm.GetSelection().GetCount());
    EXPECT_EQ("<ModelRoot/Par0>", GetPathString(0));
}

TEST_F(SelectionManagerTest, Notify) {
    Selection   last_sel;
    SMOperation last_op;

    auto sel_func = [&](const Selection &sel, SMOperation op){
        last_sel = sel;
        last_op  = op;
    };

    sm.GetSelectionChanged().AddObserver("key", sel_func);

    SelPath ppath0 = BuildSelPath(ModelVec{ root, par0 });
    SelPath ppath1 = BuildSelPath(ModelVec{ root, par1 });
    SelPath bpath0 = BuildSelPath(ModelVec{ root, par0, box0 });

    EXPECT_FALSE(last_sel.HasAny());
    sm.ChangeModelSelection(ppath0, false);
    EXPECT_EQ(1U, last_sel.GetCount());
    EXPECT_EQ("<ModelRoot/Par0>", last_sel.GetPaths()[0].ToString());
    EXPECT_ENUM_EQ(SMOperation::kSelection, last_op);

    sm.ReselectAll();
    EXPECT_EQ(1U, last_sel.GetCount());
    EXPECT_EQ("<ModelRoot/Par0>", last_sel.GetPaths()[0].ToString());
    EXPECT_ENUM_EQ(SMOperation::kReselection, last_op);

    // Deselection notification occurs before actual deselection so that the
    // caller can know what is being deselected.
    sm.DeselectAll();
    EXPECT_EQ(1U, last_sel.GetCount());
    EXPECT_EQ("<ModelRoot/Par0>", last_sel.GetPaths()[0].ToString());
    EXPECT_ENUM_EQ(SMOperation::kDeselection, last_op);

    root->AddChildModel(Model::CreateModel<BoxModel>("Box2"));
    EXPECT_EQ(0U, last_sel.GetCount());
    EXPECT_ENUM_EQ(SMOperation::kUpdate, last_op);
}
