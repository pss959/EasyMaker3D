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

    std::shared_ptr<Base_> bdp = Util::CastToBase<Base_, Derived_>(dp);
    EXPECT_NOT_NULL(bdp);
    EXPECT_EQ(dp, bdp);
    EXPECT_EQ(2,  dp.use_count());
    EXPECT_EQ(2, bdp.use_count());

    std::shared_ptr<Derived_> dbp = Util::CastToDerived<Base_, Derived_>(bp);
    EXPECT_NOT_NULL(dbp);
    EXPECT_EQ(bp, dbp);
    EXPECT_EQ(2,  bp.use_count());
    EXPECT_EQ(2, dbp.use_count());
}
