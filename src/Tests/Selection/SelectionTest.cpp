//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Selection/Selection.h"
#include "Tests/SelectionTestBase.h"
#include "Tests/Testing2.h"
#include "Util/Assert.h"

class SelectionTest : public SelectionTestBase {};

TEST_F(SelectionTest, Default) {
    Selection sel;
    EXPECT_FALSE(sel.HasAny());
    EXPECT_EQ(0U, sel.GetCount());
    TEST_ASSERT(sel.GetPrimary(), "HasAny");
    EXPECT_TRUE(sel.GetPaths().empty());
    EXPECT_TRUE(sel.GetModels().empty());
}

TEST_F(SelectionTest, PathConstructor) {
    SelPath path = BuildSelPath(ModelVec{ root, par0 });
    Selection sel(path);
    EXPECT_TRUE(sel.HasAny());
    EXPECT_EQ(1U, sel.GetCount());
    EXPECT_EQ("<ModelRoot/Par0>", sel.GetPrimary().ToString());
    EXPECT_EQ(1U, sel.GetPaths().size());
    EXPECT_EQ("<ModelRoot/Par0>", sel.GetPaths()[0].ToString());
    EXPECT_EQ(1U,   sel.GetModels().size());
    EXPECT_EQ(par0, sel.GetModels()[0]);
}

TEST_F(SelectionTest, AddAndClear) {
    SelPath p0 = BuildSelPath(ModelVec{ root, par0 });
    SelPath p1 = BuildSelPath(ModelVec{ root, par0, box0 });
    SelPath p2 = BuildSelPath(ModelVec{ root, par1, box1 });

    Selection sel;
    sel.Add(p0);
    sel.Add(p1);
    sel.Add(p2);

    EXPECT_TRUE(sel.HasAny());
    EXPECT_EQ(3U, sel.GetCount());
    EXPECT_EQ("<ModelRoot/Par0>", sel.GetPrimary().ToString());
    EXPECT_EQ(3U, sel.GetPaths().size());
    EXPECT_EQ("<ModelRoot/Par0>",      sel.GetPaths()[0].ToString());
    EXPECT_EQ("<ModelRoot/Par0/Box0>", sel.GetPaths()[1].ToString());
    EXPECT_EQ("<ModelRoot/Par1/Box1>", sel.GetPaths()[2].ToString());
    EXPECT_EQ(3U,   sel.GetModels().size());
    EXPECT_EQ(par0, sel.GetModels()[0]);
    EXPECT_EQ(box0, sel.GetModels()[1]);
    EXPECT_EQ(box1, sel.GetModels()[2]);

    sel.Clear();
    EXPECT_FALSE(sel.HasAny());
    EXPECT_EQ(0U, sel.GetCount());
    EXPECT_TRUE(sel.GetPaths().empty());
    EXPECT_TRUE(sel.GetModels().empty());
}
