#include "Tests/Testing.h"
#include "Util/Notifier.h"

TEST(NotifierTest, OneObserver) {
    const Str key = "some key";

    int count = 0;

    Util::Notifier<> n;
    EXPECT_EQ(0U, n.GetObserverCount());
    EXPECT_FALSE(n.HasObserver(key));

    // Add an observer.
    n.AddObserver(key, [&](){ ++count; });
    EXPECT_EQ(1U, n.GetObserverCount());
    EXPECT_TRUE(n.HasObserver(key));

    // Notify.
    EXPECT_EQ(0, count);
    n.Notify();
    EXPECT_EQ(1, count);
    n.Notify();
    EXPECT_EQ(2, count);

    // Disable and reenable notification.
    EXPECT_TRUE(n.IsObserverEnabled(key));
    n.EnableObserver(key, false);
    EXPECT_FALSE(n.IsObserverEnabled(key));
    n.Notify();
    EXPECT_EQ(2, count);
    n.EnableObserver(key, true);
    EXPECT_TRUE(n.IsObserverEnabled(key));
    n.Notify();
    EXPECT_EQ(3, count);

    n.EnableAll(false);
    EXPECT_TRUE(n.IsObserverEnabled(key));
    n.Notify();
    EXPECT_EQ(3, count);
    n.EnableAll(true);
    n.Notify();
    EXPECT_EQ(4, count);
}

TEST(NotifierTest, TwoObservers) {
    const Str key0 = "some key";
    const Str key1 = "another key";

    int count = 0;

    Util::Notifier<> n;
    n.AddObserver(key0, [&](){ count += 1;  });
    n.AddObserver(key1, [&](){ count += 10; });
    EXPECT_EQ(2U, n.GetObserverCount());
    EXPECT_TRUE(n.HasObserver(key0));
    EXPECT_TRUE(n.HasObserver(key1));

    // Notify.
    EXPECT_EQ(0, count);
    n.Notify();
    EXPECT_EQ(11, count);
    n.Notify();
    EXPECT_EQ(22, count);

    // Disable one observer.
    n.EnableObserver(key0, false);
    n.Notify();
    EXPECT_EQ(32, count);
    n.EnableObserver(key0, true);
    n.EnableObserver(key1, false);
    n.Notify();
    EXPECT_EQ(33, count);
    n.EnableObserver(key1, true);

    n.EnableAll(false);
    n.Notify();
    EXPECT_EQ(33, count);
    n.EnableAll(true);
    n.Notify();
    EXPECT_EQ(44, count);

    // Remove one.
    n.RemoveObserver(key0);
    EXPECT_EQ(1U, n.GetObserverCount());
    EXPECT_FALSE(n.HasObserver(key0));
    EXPECT_TRUE(n.HasObserver(key1));
    n.Notify();
    EXPECT_EQ(54, count);
}

TEST(NotifierTest, PointerKeys) {
    const int i0 = 12;
    const int i1 = 7;
    const auto key0 = &i0;
    const auto key1 = &i1;

    int count = 0;

    Util::Notifier<> n;
    n.AddObserver(key0, [&](){ count += 1;  });
    n.AddObserver(key1, [&](){ count += 10; });
    EXPECT_EQ(2U, n.GetObserverCount());
    EXPECT_TRUE(n.HasObserver(key0));
    EXPECT_TRUE(n.HasObserver(key1));

    // Notify.
    EXPECT_EQ(0, count);
    n.Notify();
    EXPECT_EQ(11, count);
    n.Notify();
    EXPECT_EQ(22, count);

    // Disable one observer.
    n.EnableObserver(key0, false);
    n.Notify();
    EXPECT_EQ(32, count);
    n.EnableObserver(key0, true);
    n.EnableObserver(key1, false);
    n.Notify();
    EXPECT_EQ(33, count);
    n.EnableObserver(key1, true);

    n.EnableAll(false);
    n.Notify();
    EXPECT_EQ(33, count);
    n.EnableAll(true);
    n.Notify();
    EXPECT_EQ(44, count);

    // Remove one.
    n.RemoveObserver(key0);
    EXPECT_EQ(1U, n.GetObserverCount());
    EXPECT_FALSE(n.HasObserver(key0));
    EXPECT_TRUE(n.HasObserver(key1));
    n.Notify();
    EXPECT_EQ(54, count);
}
