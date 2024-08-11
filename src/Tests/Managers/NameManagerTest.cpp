//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Managers/NameManager.h"
#include "Tests/Testing.h"

TEST(NameManagerTest, AddRemove) {
    const Str n1 = "Hello";
    const Str n2 = "There";
    NameManager nm;
    EXPECT_FALSE(nm.Find(n1));
    EXPECT_FALSE(nm.Find(n2));
    nm.Add(n1);
    EXPECT_TRUE(nm.Find(n1));
    EXPECT_FALSE(nm.Find(n2));
    nm.Add(n2);
    EXPECT_TRUE(nm.Find(n1));
    EXPECT_TRUE(nm.Find(n2));
    nm.Remove(n1);
    EXPECT_FALSE(nm.Find(n1));
    EXPECT_TRUE(nm.Find(n2));
    nm.Remove(n2);
    EXPECT_FALSE(nm.Find(n1));
    EXPECT_FALSE(nm.Find(n2));
}

TEST(NameManagerTest, Create) {
    const Str prefix = "Name of Doom";
    NameManager nm;
    EXPECT_FALSE(nm.Find(prefix));
    const Str n1 = nm.Create(prefix);
    EXPECT_EQ(prefix + "_1", n1);
    nm.Add(n1);
    const Str n2 = nm.Create(prefix);
    EXPECT_EQ(prefix + "_2", n2);
    nm.Add(n2);
    EXPECT_TRUE(nm.Find(n1));
    EXPECT_TRUE(nm.Find(n2));
}

TEST(NameManagerTest, Reset) {
    const Str n1 = "Hello";
    const Str n2 = "There";
    NameManager nm;
    nm.Add(n1);
    nm.Add(n2);
    EXPECT_TRUE(nm.Find(n1));
    EXPECT_TRUE(nm.Find(n2));
    nm.Reset();
    EXPECT_FALSE(nm.Find(n1));
    EXPECT_FALSE(nm.Find(n2));
}

TEST(NameManagerTest, CreateClone) {
    const Str name = "Squid";
    NameManager nm;
    nm.Add(name);
    EXPECT_TRUE(nm.Find(name));
    EXPECT_FALSE(nm.Find(name + "_A"));
    EXPECT_FALSE(nm.Find(name + "_B"));
    EXPECT_FALSE(nm.Find(name + "_C"));
    const Str n1 = nm.CreateClone(name);
    EXPECT_EQ(name + "_A", n1);
    nm.Add(n1);
    const Str n2 = nm.CreateClone(name);
    EXPECT_EQ(name + "_B", n2);
    nm.Add(n2);
    const Str n3 = nm.CreateClone(name);
    EXPECT_EQ(name + "_C", n3);
}

TEST(NameManagerTest, GetAllNames) {
    NameManager nm;
    nm.Add("This is");
    nm.Add("A fun test");
    nm.Add("For sure");
    const StrVec names = nm.GetAllNames();
    EXPECT_EQ(3U, names.size());
    EXPECT_EQ("A fun test", names[0]);
    EXPECT_EQ("For sure",   names[1]);
    EXPECT_EQ("This is",    names[2]);
}
