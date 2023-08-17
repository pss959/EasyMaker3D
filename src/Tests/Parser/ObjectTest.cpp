#include "Tests/Parser/ParserTestBase.h"
#include "Util/String.h"

/// \ingroup Tests
class ObjectTest : public ParserTestBase {};

TEST_F(ObjectTest, Default) {
    auto dp = Parser::Registry::CreateObject<Derived>();
    EXPECT_EQ("Derived", dp->GetTypeName());
    EXPECT_FALSE(dp->IsTemplate());
    EXPECT_FALSE(dp->IsClone());
    EXPECT_TRUE(dp->IsCreationDone2());
}

TEST_F(ObjectTest, Fields) {
    auto dp = Parser::Registry::CreateObject<Derived>();
    EXPECT_NOT_NULL(dp->FindField("str_val"));
    EXPECT_NULL(dp->FindField("no_such_field"));
    EXPECT_FALSE(dp->WasAnyFieldSet());
    dp->int_val = 123;
    EXPECT_TRUE(dp->WasAnyFieldSet());
}

TEST_F(ObjectTest, GetNameAndDesc) {
    auto dp1 = Parser::Registry::CreateObject<Derived>();
    auto dp2 = Parser::Registry::CreateObject<Derived>("Buddy");

    const Str addr1 = Util::ToString(dp1.get());
    const Str addr2 = Util::ToString(dp2.get());
    const Str desc1 = "Derived ("         + addr1 + ")";
    const Str desc2 = "Derived 'Buddy' (" + addr2 + ")";

    EXPECT_EQ("",      dp1->GetName());
    EXPECT_EQ("Buddy", dp2->GetName());
    EXPECT_EQ(desc1,   dp1->GetDesc());
    EXPECT_EQ(desc1,   dp1->ToString());
    EXPECT_EQ(desc2,   dp2->GetDesc());
    EXPECT_EQ(desc2,   dp2->ToString());
}

TEST_F(ObjectTest, IsScoped) {
    auto dp = Parser::Registry::CreateObject<Derived>();
    auto up = Parser::Registry::CreateObject<Unscoped>();
    EXPECT_TRUE(dp->IsScoped());
    EXPECT_FALSE(up->IsScoped());
}
