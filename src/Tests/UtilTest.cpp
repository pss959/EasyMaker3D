#include <vector>

#include "Testing.h"
#include "Util/General.h"

// Dummy classes for testing casts.
class Base_ {
  public:
    virtual ~Base_() {}  // Makes it polymorphic.
};
class Derived_ : public Base_ {
};

TEST(UtilTest, Casts) {
    // Make sure basic casts work.
    {
        Derived_ d1;
        Derived_ d2;
        Base_    *bp = &d1;
        Derived_ *dp = &d2;
        EXPECT_EQ(&d1, dynamic_cast<Derived_ *>(bp));
        EXPECT_EQ(&d2, dynamic_cast<Base_ *>(dp));
    }

    std::shared_ptr<Base_>    bp(new Derived_);
    std::shared_ptr<Derived_> dp(new Derived_);
    EXPECT_EQ(1, bp.use_count());
    EXPECT_EQ(1, dp.use_count());

    std::shared_ptr<Base_> bdp = Util::CastToBase<Base_>(dp);
    EXPECT_NOT_NULL(bdp);
    EXPECT_EQ(dp, bdp);
    EXPECT_EQ(2,  dp.use_count());
    EXPECT_EQ(2, bdp.use_count());

    std::shared_ptr<Derived_> dbp = Util::CastToDerived<Derived_>(bp);
    EXPECT_NOT_NULL(dbp);
    EXPECT_EQ(bp, dbp);
    EXPECT_EQ(2,  bp.use_count());
    EXPECT_EQ(2, dbp.use_count());
}

TEST(UtilTest, ConvertVector) {
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

TEST(UtilTest, EraseIf) {
    std::vector<int> ints{ 1, 2, 3, 4, 5 };
    Util::EraseIf(ints, [](int i){ return i % 2 == 1; });
    EXPECT_EQ(2U, ints.size());
    EXPECT_EQ(2, ints[0]);
    EXPECT_EQ(4, ints[1]);
}

