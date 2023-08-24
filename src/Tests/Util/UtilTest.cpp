#include <vector>

#include "Tests/Testing.h"
#include "Util/General.h"
#include "Util/URL.h"

/// \ingroup Tests
class UtilTest : public ::testing::Test {
  protected:
    /// Dummy base class for testing casts.
    class Base {
      public:
        virtual ~Base() {}  // Makes it polymorphic.
    };

    /// Dummy derived class for testing casts.
    class Derived : public Base {};
};

TEST_F(UtilTest, IsInUnitTest) {
    EXPECT_EQ(Util::AppType::kUnitTest, Util::app_type);
}

TEST_F(UtilTest, Casts) {
    // Make sure basic casts work.
    {
        Derived d1;
        Derived d2;
        Base    *bp = &d1;
        Derived *dp = &d2;
        EXPECT_EQ(&d1, dynamic_cast<Derived *>(bp));
        EXPECT_EQ(&d2, dynamic_cast<Base *>(dp));
    }

    std::shared_ptr<Base>    bp(new Derived);
    std::shared_ptr<Derived> dp(new Derived);
    EXPECT_EQ(1, bp.use_count());
    EXPECT_EQ(1, dp.use_count());

    std::shared_ptr<Base> bdp = std::dynamic_pointer_cast<Base>(dp);
    EXPECT_NOT_NULL(bdp);
    EXPECT_EQ(dp, bdp);
    EXPECT_EQ(2,  dp.use_count());
    EXPECT_EQ(2, bdp.use_count());

    std::shared_ptr<Derived> dbp = std::dynamic_pointer_cast<Derived>(bp);
    EXPECT_NOT_NULL(dbp);
    EXPECT_EQ(bp, dbp);
    EXPECT_EQ(2,  bp.use_count());
    EXPECT_EQ(2, dbp.use_count());
}

TEST_F(UtilTest, IsA) {
    std::shared_ptr<Base>    bp(new Derived);
    std::shared_ptr<Derived> dp(new Derived);
    EXPECT_TRUE(Util::IsA<Base>(bp));
    EXPECT_TRUE(Util::IsA<Base>(dp));
    EXPECT_TRUE(Util::IsA<Derived>(bp));
    EXPECT_TRUE(Util::IsA<Derived>(bp));
}

TEST_F(UtilTest, Contains) {
    const std::vector<int> v{0, 2, 4, 6, 8};
    EXPECT_TRUE(Util::Contains(v, 0));
    EXPECT_TRUE(Util::Contains(v, 2));
    EXPECT_TRUE(Util::Contains(v, 8));
    EXPECT_FALSE(Util::Contains(v, 1));
    EXPECT_FALSE(Util::Contains(v, 9));
}

TEST_F(UtilTest, GetKeysAndValues) {
    std::unordered_map<Str, int> map;
    map["hello"] = 13;
    map["abc"]   = 9;
    map["squid"] = 31;
    map["blort"] = 5;

    StrVec keys = Util::GetKeys(map);
    std::sort(keys.begin(), keys.end());
    EXPECT_EQ("abc",   keys[0]);
    EXPECT_EQ("blort", keys[1]);
    EXPECT_EQ("hello", keys[2]);
    EXPECT_EQ("squid", keys[3]);

    std::vector<int> values = Util::GetValues(map);
    std::sort(values.begin(), values.end());
    EXPECT_EQ(5,  values[0]);
    EXPECT_EQ(9,  values[1]);
    EXPECT_EQ(13, values[2]);
    EXPECT_EQ(31, values[3]);
}

TEST_F(UtilTest, FindAll) {
    const std::vector<int> ints{ 1, 2, 3, 4, 5, 6, 7, 8 };
    const std::vector<int> evens =
        Util::FindAll<int>(ints, [](const int &i){ return i % 2 == 0; });
    EXPECT_EQ(4U, evens.size());
    EXPECT_EQ(2, evens[0]);
    EXPECT_EQ(4, evens[1]);
    EXPECT_EQ(6, evens[2]);
    EXPECT_EQ(8, evens[3]);
}

TEST_F(UtilTest, ConvertVector) {
    std::vector<int> ints{ 1, 2, 3, 4, 5 };
    std::vector<float> floats = Util::ConvertVector<float, int>(
        ints, [](const int &i){ return .5f * i; });
    EXPECT_EQ(5U, floats.size());
    EXPECT_EQ(0.5f, floats[0]);
    EXPECT_EQ(1.0f, floats[1]);
    EXPECT_EQ(1.5f, floats[2]);
    EXPECT_EQ(2.0f, floats[3]);
    EXPECT_EQ(2.5f, floats[4]);
}

TEST_F(UtilTest, AppendVector) {
    std::vector<int>       v0{ 1, 2 };
    const std::vector<int> v1{ 3, 4, 5 };
    Util::AppendVector(v1, v0);
    EXPECT_EQ(5U, v0.size());
    EXPECT_EQ(1, v0[0]);
    EXPECT_EQ(2, v0[1]);
    EXPECT_EQ(3, v0[2]);
    EXPECT_EQ(4, v0[3]);
    EXPECT_EQ(5, v0[4]);
}

TEST_F(UtilTest, CreateTemporarySharedPtr) {
    std::shared_ptr<Derived> dp(new Derived);
    EXPECT_EQ(1, dp.use_count());
    {
        // tp should not affect the reference count.
        auto tp = Util::CreateTemporarySharedPtr<>(dp.get());
        EXPECT_EQ(1, dp.use_count());
    }
    EXPECT_EQ(1, dp.use_count());
}

TEST_F(UtilTest, OpenURL) {
    Str last_url;
    Util::SetOpenURLFunc([&](const Str &url){ last_url = url; });

    EXPECT_EQ("", last_url);

    Str url = "https://this.iswrong.com/bad.html";
    Util::OpenURL(url);
    EXPECT_EQ(url, last_url);

    url = "http://some.other.org/weird.html";
    Util::OpenURL(url);
    EXPECT_EQ(url, last_url);
}
