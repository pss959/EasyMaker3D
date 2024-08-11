//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Selection/SelPath.h"
#include "Tests/SelectionTestBase.h"
#include "Tests/Testing2.h"
#include "Util/Assert.h"

class SelPathTest : public SelectionTestBase {};

TEST_F(SelPathTest, Default) {
    SelPath path;
    EXPECT_TRUE(path.empty());
    TEST_ASSERT(path.Validate(),       "empty");
    TEST_ASSERT(path.GetModel(),       "empty");
    TEST_ASSERT(path.GetParentModel(), "size");
}

TEST_F(SelPathTest, WithRootAndChild) {
    // Constructor taking root and top-level child.
    SelPath path(root, par0);
    EXPECT_EQ(2U, path.size());
    EXPECT_EQ(root, path[0]);
    EXPECT_EQ(par0, path[1]);
    path.Validate();  // Should not throw.
    EXPECT_EQ(par0, path.GetModel());
    EXPECT_EQ(root, path.GetParentModel());

    auto models = path.GetAllModels(false);   // Include root.
    EXPECT_EQ(2U,   models.size());
    EXPECT_EQ(root, models[0]);
    EXPECT_EQ(par0, models[1]);

    models = path.GetAllModels(true);         // Skip root.
    EXPECT_EQ(1U,   models.size());
    EXPECT_EQ(par0, models[0]);
}

TEST_F(SelPathTest, FromNodePath) {
    // Anything above the RootModel is ignored.
    SG::NodePath npath;
    npath.push_back(top);
    npath.push_back(root);
    npath.push_back(par0);

    SelPath path(npath);
    EXPECT_EQ(2U, path.size());
    EXPECT_EQ(root, path[0]);
    EXPECT_EQ(par0, path[1]);
    path.Validate();  // Should not throw.
    EXPECT_EQ(par0, path.GetModel());
    EXPECT_EQ(root, path.GetParentModel());

    auto models = path.GetAllModels(false);  // Include root.
    EXPECT_EQ(2U,   models.size());
    EXPECT_EQ(root, models[0]);
    EXPECT_EQ(par0, models[1]);

    models = path.GetAllModels(true);        // Skip root.
    EXPECT_EQ(1U,   models.size());
    EXPECT_EQ(par0, models[0]);
}

TEST_F(SelPathTest, FromNodePath2) {
    // Anything above the RootModel is ignored.
    SG::NodePath npath;
    npath.push_back(top);
    npath.push_back(root);
    npath.push_back(par0);
    npath.push_back(box1);

    SelPath path(npath);
    EXPECT_EQ(3U, path.size());
    EXPECT_EQ(root, path[0]);
    EXPECT_EQ(par0, path[1]);
    EXPECT_EQ(box1, path[2]);
    path.Validate();  // Should not throw.
    EXPECT_EQ(box1, path.GetModel());
    EXPECT_EQ(par0, path.GetParentModel());

    auto models = path.GetAllModels(false);  // Include root.
    EXPECT_EQ(3U,   models.size());
    EXPECT_EQ(root, models[0]);
    EXPECT_EQ(par0, models[1]);
    EXPECT_EQ(box1, models[2]);

    models = path.GetAllModels(true);        // Skip root.
    EXPECT_EQ(2U,   models.size());
    EXPECT_EQ(par0, models[0]);
    EXPECT_EQ(box1, models[1]);
}

TEST_F(SelPathTest, IsAncestorOf) {
    SelPath rtpath  = BuildSelPath(ModelVec{ root });
    SelPath ppath0 = BuildSelPath(ModelVec{ root, par0 });
    SelPath ppath1 = BuildSelPath(ModelVec{ root, par1 });
    SelPath bpath0 = BuildSelPath(ModelVec{ root, par0, box0 });
    SelPath bpath1 = BuildSelPath(ModelVec{ root, par1, box0 });

    // No SelPath is an ancestor of itself.
    EXPECT_FALSE(rtpath.IsAncestorOf(rtpath));
    EXPECT_FALSE(ppath0.IsAncestorOf(ppath0));
    EXPECT_FALSE(ppath1.IsAncestorOf(ppath1));
    EXPECT_FALSE(bpath0.IsAncestorOf(bpath0));
    EXPECT_FALSE(bpath1.IsAncestorOf(bpath1));

    // Valid ancestors.
    EXPECT_TRUE(rtpath.IsAncestorOf(ppath0));
    EXPECT_TRUE(rtpath.IsAncestorOf(ppath1));
    EXPECT_TRUE(ppath0.IsAncestorOf(bpath0));
    EXPECT_TRUE(ppath1.IsAncestorOf(bpath1));
    EXPECT_TRUE(rtpath.IsAncestorOf(bpath0));
    EXPECT_TRUE(rtpath.IsAncestorOf(bpath1));

    // Invalid ancestors.
    EXPECT_FALSE(bpath0.IsAncestorOf(ppath0));
    EXPECT_FALSE(bpath0.IsAncestorOf(ppath1));
    EXPECT_FALSE(bpath0.IsAncestorOf(rtpath));
    EXPECT_FALSE(bpath1.IsAncestorOf(ppath0));
    EXPECT_FALSE(bpath1.IsAncestorOf(ppath1));
    EXPECT_FALSE(bpath1.IsAncestorOf(rtpath));
    EXPECT_FALSE(ppath0.IsAncestorOf(bpath1));
    EXPECT_FALSE(ppath0.IsAncestorOf(ppath1));
    EXPECT_FALSE(ppath0.IsAncestorOf(rtpath));
    EXPECT_FALSE(ppath1.IsAncestorOf(bpath0));
    EXPECT_FALSE(ppath1.IsAncestorOf(ppath0));
    EXPECT_FALSE(ppath1.IsAncestorOf(rtpath));
}

TEST_F(SelPathTest, GetCoordConv) {
    root->SetTranslation(Vector3f(10, 20, 30));
    par0->SetUniformScale(4);

    SelPath path = BuildSelPath(ModelVec{ root, par0 });
    auto cc = path.GetCoordConv();

    EXPECT_EQ(Point3f(10, 20, 30), cc.ObjectToRoot(Point3f(0, 0, 0)));
    EXPECT_EQ(Point3f(14, 24, 34), cc.ObjectToRoot(Point3f(1, 1, 1)));
}

TEST_F(SelPathTest, GetPathToChild) {
    SelPath path = BuildSelPath(ModelVec{ root, par0 });

    SelPath cpath = path.GetPathToChild(box0);
    EXPECT_EQ(3U, cpath.size());
    EXPECT_EQ(root, cpath[0]);
    EXPECT_EQ(par0, cpath[1]);
    EXPECT_EQ(box0, cpath[2]);

    cpath = path.GetPathToChild(box1);
    EXPECT_EQ(3U, cpath.size());
    EXPECT_EQ(root, cpath[0]);
    EXPECT_EQ(par0, cpath[1]);
    EXPECT_EQ(box1, cpath[2]);
}
