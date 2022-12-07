#include "Managers/NameManager.h"
#include "Tests/Testing.h"

TEST(NameManager, AddRemove) {
    const std::string n1 = "Hello";
    const std::string n2 = "There";
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

TEST(NameManager, Create) {
    const std::string prefix = "Name of Doom";
    NameManager nm;
    EXPECT_FALSE(nm.Find(prefix));
    const std::string n1 = nm.Create(prefix);
    EXPECT_EQ(prefix + "_1", n1);
    nm.Add(n1);
    const std::string n2 = nm.Create(prefix);
    EXPECT_EQ(prefix + "_2", n2);
    nm.Add(n2);
    EXPECT_TRUE(nm.Find(n1));
    EXPECT_TRUE(nm.Find(n2));
}

TEST(NameManager, Reset) {
    const std::string n1 = "Hello";
    const std::string n2 = "There";
    NameManager nm;
    nm.Add(n1);
    nm.Add(n2);
    EXPECT_TRUE(nm.Find(n1));
    EXPECT_TRUE(nm.Find(n2));
    nm.Reset();
    EXPECT_FALSE(nm.Find(n1));
    EXPECT_FALSE(nm.Find(n2));
}

TEST(NameManager, CreateClone) {
    const std::string name = "Squid";
    NameManager nm;
    nm.Add(name);
    EXPECT_TRUE(nm.Find(name));
    EXPECT_FALSE(nm.Find(name + "_A"));
    EXPECT_FALSE(nm.Find(name + "_B"));
    EXPECT_FALSE(nm.Find(name + "_C"));
    const std::string n1 = nm.CreateClone(name);
    EXPECT_EQ(name + "_A", n1);
    nm.Add(n1);
    const std::string n2 = nm.CreateClone(name);
    EXPECT_EQ(name + "_B", n2);
    nm.Add(n2);
    const std::string n3 = nm.CreateClone(name);
    EXPECT_EQ(name + "_C", n3);
}

TEST(NameManager, GetAllNames) {
    NameManager nm;
    nm.Add("This is");
    nm.Add("A fun test");
    nm.Add("For sure");
    const std::vector<std::string> names = nm.GetAllNames();
    EXPECT_EQ(3U, names.size());
    EXPECT_EQ("A fun test", names[0]);
    EXPECT_EQ("For sure",   names[1]);
    EXPECT_EQ("This is",    names[2]);
}
