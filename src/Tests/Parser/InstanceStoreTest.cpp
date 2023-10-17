#include "Parser/InstanceStore.h"
#include "Tests/Parser/ParserTestBase.h"
#include "Util/Assert.h"

/// \ingroup Tests
class InstanceStoreTest : public ParserTestBase {};

TEST_F(InstanceStoreTest, Store) {
    Parser::InstanceStore store;

    auto orig_sp = Parser::Registry::CreateObject<Simple>();
    auto orig_dp = Parser::Registry::CreateObject<Derived>();

    EXPECT_FALSE(store.HasOriginal<Simple>());
    EXPECT_FALSE(store.HasOriginal<Derived>());

    store.AddOriginal(orig_sp);
    EXPECT_TRUE(store.HasOriginal<Simple>());
    EXPECT_FALSE(store.HasOriginal<Derived>());

    store.AddOriginal(orig_dp);
    EXPECT_TRUE(store.HasOriginal<Simple>());
    EXPECT_TRUE(store.HasOriginal<Derived>());

    auto sp0 = store.Acquire<Simple>();
    auto sp1 = store.Acquire<Simple>();
    auto dp0 = store.Acquire<Derived>();
    auto dp1 = store.Acquire<Derived>();
    EXPECT_NOT_NULL(sp0.get());
    EXPECT_NOT_NULL(sp1.get());
    EXPECT_NOT_NULL(dp0.get());
    EXPECT_NOT_NULL(dp1.get());
    EXPECT_EQ("ParserTestBase::Simple_0",  sp0->GetName());
    EXPECT_EQ("ParserTestBase::Simple_1",  sp1->GetName());
    EXPECT_EQ("ParserTestBase::Derived_0", dp0->GetName());
    EXPECT_EQ("ParserTestBase::Derived_1", dp1->GetName());

    // Release and re-acquire should return the same instance.
    store.Release(sp0);
    EXPECT_EQ(sp0, store.Acquire<Simple>());

    store.Release(sp0);
    store.Release(sp1);
    auto spa = store.Acquire<Simple>();
    auto spb = store.Acquire<Simple>();
    EXPECT_TRUE((spa == sp0 && spb == sp1) ||
                (spa == sp1 && spb == sp0));

    // Cannot acquire an instance for an uninitialized class.
    TEST_ASSERT(store.Acquire<Other>(), "contains");

    store.Reset();
    EXPECT_FALSE(store.HasOriginal<Simple>());
    EXPECT_FALSE(store.HasOriginal<Derived>());
}
